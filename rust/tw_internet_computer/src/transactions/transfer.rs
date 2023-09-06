use std::time::Duration;

use ic_certification::Label;
use tw_keypair::ecdsa::secp256k1::PrivateKey;

use crate::{
    address::AccountIdentifier,
    protocol::{
        envelope::{Envelope, EnvelopeContent},
        get_ingress_expiry,
        identity::Identity,
        principal::Principal,
        request_id::RequestId,
        rosetta,
    },
    transactions::proto::ic_ledger::pb::v1::{
        AccountIdentifier as ProtoAccountIdentifier, Memo, Payment, SendRequest, TimeStamp, Tokens,
    },
};

use super::SignTransactionError;

#[derive(Clone, Debug)]
pub struct TransferArgs {
    pub memo: u64,
    pub amount: u64,
    pub max_fee: Option<u64>,
    pub to: String,
    pub current_timestamp_nanos: u64,
}

impl TryFrom<TransferArgs> for SendRequest<'_> {
    type Error = SignTransactionError;

    fn try_from(args: TransferArgs) -> Result<Self, Self::Error> {
        let current_timestamp_duration = Duration::from_nanos(args.current_timestamp_nanos);
        let timestamp_nanos = current_timestamp_duration.as_nanos() as u64;

        let to_account_identifier = AccountIdentifier::from_hex(&args.to)
            .map_err(|_| SignTransactionError::InvalidToAccountIdentifier)?;
        let to_hash = to_account_identifier.as_ref().to_vec();

        let request = Self {
            memo: Some(Memo { memo: args.memo }),
            payment: Some(Payment {
                receiver_gets: Some(Tokens { e8s: args.amount }),
            }),
            max_fee: args.max_fee.map(|fee| Tokens { e8s: fee }),
            from_subaccount: None,
            to: Some(ProtoAccountIdentifier {
                hash: to_hash.into(),
            }),
            created_at: None,
            created_at_time: Some(TimeStamp { timestamp_nanos }),
        };
        Ok(request)
    }
}

/// The endpoint on the ledger canister that is used to make transfers.
const METHOD_NAME: &str = "send_pb";

pub fn transfer(
    private_key: PrivateKey,
    canister_id: Principal,
    args: TransferArgs,
) -> Result<rosetta::SignedTransaction, SignTransactionError> {
    if args.amount < 1 {
        return Err(SignTransactionError::InvalidAmount);
    }

    let current_timestamp_duration = Duration::from_nanos(args.current_timestamp_nanos);
    let ingress_expiry = get_ingress_expiry(current_timestamp_duration);
    let identity = Identity::new(private_key);

    // Encode the arguments for the ledger `send_pb` endpoint.
    let send_request = SendRequest::try_from(args)?;
    let arg =
        tw_proto::serialize(&send_request).map_err(|_| SignTransactionError::EncodingArgsFailed)?;
    // Create the update envelope.
    let (request_id, update_envelope) =
        create_update_envelope(&identity, canister_id, arg, ingress_expiry)?;

    // Create the read state envelope.
    let (_, read_state_envelope) =
        create_read_state_envelope(&identity, request_id, ingress_expiry)?;

    // Create a new EnvelopePair with the update call and read_state envelopes.
    let envelope_pair = rosetta::EnvelopePair::new(update_envelope, read_state_envelope);

    // Create a signed transaction containing the envelope pair.
    let request: rosetta::Request = (rosetta::RequestType::Send, vec![envelope_pair]);
    Ok(vec![request])
}

#[inline]
fn create_update_envelope(
    identity: &Identity,
    canister_id: Principal,
    arg: Vec<u8>,
    ingress_expiry: u64,
) -> Result<(RequestId, Envelope), SignTransactionError> {
    let sender = identity.sender();
    let content = EnvelopeContent::Call {
        nonce: None, //TODO: do we need the nonce?
        ingress_expiry,
        sender,
        canister_id,
        method_name: METHOD_NAME.to_string(),
        arg,
    };

    let request_id = RequestId::from(&content);
    let signature = identity
        .sign(request_id.sig_data())
        .map_err(SignTransactionError::Identity)?;

    let env = Envelope {
        content,
        sender_pubkey: Some(signature.public_key),
        sender_sig: Some(signature.signature),
    };
    Ok((request_id, env))
}

#[inline]
fn create_read_state_envelope(
    identity: &Identity,
    update_request_id: RequestId,
    ingress_expiry: u64,
) -> Result<(RequestId, Envelope), SignTransactionError> {
    let sender = identity.sender();

    let content = EnvelopeContent::ReadState {
        ingress_expiry,
        sender,
        paths: vec![vec![
            Label::from("request_status"),
            Label::from(update_request_id.0.as_slice()),
        ]],
    };

    let request_id = RequestId::from(&content);
    let signature = identity
        .sign(request_id.sig_data())
        .map_err(SignTransactionError::Identity)?;

    let env = Envelope {
        content,
        sender_pubkey: Some(signature.public_key),
        sender_sig: Some(signature.signature),
    };
    Ok((request_id, env))
}

#[cfg(test)]
mod test {
    use tw_encoding::hex;

    use crate::address::AccountIdentifier;

    use super::*;

    pub const SIGNED_TRANSACTION: &str = "81826b5452414e53414354494f4e81a266757064617465a367636f6e74656e74a66c726571756573745f747970656463616c6c6e696e67726573735f6578706972791b177a297215cfe8006673656e646572581d971cd2ddeecd1cf1b28be914d7a5c43441f6296f1f9966a7c8aff68d026b63616e69737465725f69644a000000000000000201016b6d6574686f645f6e616d656773656e645f706263617267583b0a0012070a050880c2d72f2a220a20943d12e762f43806782f524b8f90297298a6d79e4749b41b585ec427409c826a3a0a088090caa5a3a78abd176d73656e6465725f7075626b65799858183018561830100607182a1886184818ce183d02010605182b188104000a0318420004183d18ab183a182118a81838184d184c187e1852188a187e18dc18d8184418ea18cd18c5189518ac188518b518bc181d188515186318bc18e618ab18d2184318d3187c184f18cd18f018de189b18b5181918dd18ef1889187218e71518c40418d4189718881843187218c611182e18cc18e6186b182118630218356a73656e6465725f736967984013186f18b9181c189818b318a8186518b2186118d418971618b1187d18eb185818e01826182f1873183b185018cb185d18ef18d81839186418b3183218da1824182f184e18a01880182718c0189018c918a018fd18c418d9189e189818b318ef1874183b185118e118a51864185918e718ed18c71889186c1822182318ca6a726561645f7374617465a367636f6e74656e74a46c726571756573745f747970656a726561645f73746174656e696e67726573735f6578706972791b177a297215cfe8006673656e646572581d971cd2ddeecd1cf1b28be914d7a5c43441f6296f1f9966a7c8aff68d0265706174687381824e726571756573745f7374617475735820e8fbc2d5b0bf837b3a369249143e50d4476faafb2dd620e4e982586a51ebcf1e6d73656e6465725f7075626b65799858183018561830100607182a1886184818ce183d02010605182b188104000a0318420004183d18ab183a182118a81838184d184c187e1852188a187e18dc18d8184418ea18cd18c5189518ac188518b518bc181d188515186318bc18e618ab18d2184318d3187c184f18cd18f018de189b18b5181918dd18ef1889187218e71518c40418d4189718881843187218c611182e18cc18e6186b182118630218356a73656e6465725f7369679840182d182718201888188618ce187f0c182a187a18d718e818df18fb18d318d41118a5186a184b18341842185318d718e618e8187a1828186c186a183618461418f3183318bd18a618a718bc18d918c818b7189d186e1865188418ff18fd18e418e9187f181b18d705184818b21872187818d6181c161833184318a2";

    #[test]
    fn transfer_successful() {
        let current_timestamp_nanos = Duration::from_secs(1_691_709_940).as_nanos() as u64;
        println!("ctn: {}", current_timestamp_nanos);
        let private_key = PrivateKey::try_from(
            "227102911bb99ce7285a55f952800912b7d22ebeeeee59d77fc33a5d7c7080be",
        )
        .unwrap();
        let canister_id = Principal::from_text("ryjl3-tyaaa-aaaaa-aaaba-cai").unwrap();
        let owner =
            Principal::from_text("t4u4z-y3dur-j63pk-nw4rv-yxdbt-agtt6-nygn7-ywh6y-zm2f4-sdzle-3qe")
                .unwrap();
        let to_account_identifier = AccountIdentifier::new(&owner);

        let signed_transaction = transfer(
            private_key,
            canister_id,
            TransferArgs {
                memo: 0,
                amount: 100_000_000,
                max_fee: None,
                to: to_account_identifier.to_hex(),
                current_timestamp_nanos,
            },
        )
        .unwrap();
        // Encode the signed transaction.
        let cbor_encoded_signed_transaction = tw_cbor::serialize(&signed_transaction).unwrap();
        let hex_encoded_signed_transaction = hex::encode(&cbor_encoded_signed_transaction, false);
        assert_eq!(hex_encoded_signed_transaction, SIGNED_TRANSACTION);
    }
}
