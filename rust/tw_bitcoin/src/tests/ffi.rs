use crate::brc20::{BRC20TransferInscription, Ticker};
use crate::modules::legacy::*;
use bitcoin::{PublicKey, ScriptBuf};
use secp256k1::XOnlyPublicKey;
use std::ffi::CString;
use tw_encoding::hex;
use tw_proto::Bitcoin::Proto as LegacyProto;

const SATOSHIS: i64 = 0;
const PUBKEY: &str = "028d7dce6d72fb8f7af9566616c6436349c67ad379f2404dd66fe7085fe0fba28f";

#[test]
fn ffi_tw_build_p2pkh_script() {
    let pubkey_slice = hex::decode(PUBKEY).unwrap();
    let pubkey = PublicKey::from_slice(&pubkey_slice).unwrap();

    let raw = unsafe {
        tw_build_p2pkh_script(SATOSHIS, pubkey_slice.as_ptr(), pubkey_slice.len()).into_vec()
    };

    // The expected script.
    let expected = ScriptBuf::new_p2pkh(&pubkey.pubkey_hash());

    let proto: LegacyProto::TransactionOutput = tw_proto::deserialize(&raw).unwrap();
    assert_eq!(proto.value, SATOSHIS);
    assert_eq!(proto.script, expected.as_bytes());
    assert!(proto.spendingScript.is_empty());
}

#[test]
fn ffi_tw_build_p2wpkh_script() {
    let pubkey_slice = hex::decode(PUBKEY).unwrap();
    let pubkey = PublicKey::from_slice(&pubkey_slice).unwrap();

    let raw = unsafe {
        tw_build_p2wpkh_script(SATOSHIS, pubkey_slice.as_ptr(), pubkey_slice.len()).into_vec()
    };

    // The expected script.
    let expected = ScriptBuf::new_v0_p2wpkh(&pubkey.wpubkey_hash().unwrap());

    let proto: LegacyProto::TransactionOutput = tw_proto::deserialize(&raw).unwrap();
    assert_eq!(proto.value, SATOSHIS);
    assert_eq!(proto.script, expected.as_bytes());
    assert!(proto.spendingScript.is_empty());
}

#[test]
fn ffi_tw_build_p2tr_key_path_script() {
    let pubkey_slice = hex::decode(PUBKEY).unwrap();
    let pubkey = PublicKey::from_slice(&pubkey_slice).unwrap();

    let raw = unsafe {
        tw_build_p2tr_key_path_script(SATOSHIS, pubkey_slice.as_ptr(), pubkey_slice.len())
            .into_vec()
    };

    // The expected script.
    let xonly = XOnlyPublicKey::from(pubkey.inner);
    let expected = ScriptBuf::new_v1_p2tr(&secp256k1::Secp256k1::new(), xonly, None);

    let proto: LegacyProto::TransactionOutput = tw_proto::deserialize(&raw).unwrap();
    assert_eq!(proto.value, SATOSHIS);
    assert_eq!(proto.script, expected.as_bytes());
    assert!(proto.spendingScript.is_empty());
}

#[test]
fn ffi_tw_build_brc20_transfer_inscription() {
    let pubkey_slice = hex::decode(PUBKEY).unwrap();
    let pubkey = PublicKey::from_slice(&pubkey_slice).unwrap();

    let ticker_str = "oadf";
    let c_ticker = CString::new(ticker_str).unwrap();
    let brc20_amount = 100;

    // Call the FFI function.
    let raw = unsafe {
        tw_build_brc20_transfer_inscription(
            c_ticker.as_ptr(),
            brc20_amount,
            SATOSHIS,
            pubkey_slice.as_ptr(),
            pubkey_slice.len(),
        )
        .into_vec()
    };

    // Prepare the BRC20 payload + merkle root.
    let ticker = Ticker::new(ticker_str.to_string()).unwrap();
    let transfer = BRC20TransferInscription::new(pubkey.into(), ticker, brc20_amount).unwrap();

    let merkle_root = transfer
        .inscription()
        .spend_info()
        .merkle_root()
        .expect("incorrectly constructed Taproot merkle root");

    // The expected script.
    let xonly = XOnlyPublicKey::from(pubkey.inner);
    let expected = ScriptBuf::new_v1_p2tr(&secp256k1::Secp256k1::new(), xonly, Some(merkle_root));

    let proto: LegacyProto::TransactionOutput = tw_proto::deserialize(&raw).unwrap();
    assert_eq!(proto.value, SATOSHIS);
    assert_eq!(proto.script, expected.as_bytes());
    assert_eq!(
        proto.spendingScript,
        transfer.inscription().taproot_program().as_bytes()
    );
}

#[test]
fn ffi_tw_bitcoin_build_nft_inscription() {
    //TODO...
}
