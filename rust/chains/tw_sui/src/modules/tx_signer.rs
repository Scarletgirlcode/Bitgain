// SPDX-License-Identifier: Apache-2.0
//
// Copyright © 2017 Trust Wallet.

use crate::address::SuiAddress;
use crate::signature::SuiSignatureInfo;
use crate::transaction::transaction_data::TransactionData;
use serde::Serialize;
use serde_repr::Serialize_repr;
use tw_coin_entry::error::{SigningError, SigningErrorType, SigningResult};
use tw_encoding::bcs;
use tw_hash::blake2::blake2_b;
use tw_hash::H256;
use tw_keypair::ed25519;
use tw_keypair::traits::{KeyPairTrait, SigningKeyTrait};
use tw_memory::Data;

/// This enums specifies the intent scope.
#[derive(Serialize_repr)]
#[repr(u8)]
pub enum IntentScope {
    /// Used for a user signature on a transaction data.
    TransactionData = 0,
}

/// The version here is to distinguish between signing different versions of the struct
/// or enum. Serialized output between two different versions of the same struct/enum
/// might accidentally (or maliciously on purpose) match.
#[derive(Serialize_repr)]
#[repr(u8)]
pub enum IntentVersion {
    V0 = 0,
}

/// This enums specifies the application ID. Two intents in two different applications
/// (i.e., Narwhal, Sui, Ethereum etc) should never collide, so that even when a signing
/// key is reused, nobody can take a signature designated for app_1 and present it as a
/// valid signature for an (any) intent in app_2.
#[derive(Serialize_repr)]
#[repr(u8)]
pub enum AppId {
    Sui = 0,
}

/// An intent is a compact struct serves as the domain separator for a message that a signature commits to.
/// It consists of three parts: [enum IntentScope] (what the type of the message is),
/// [enum IntentVersion], [enum AppId] (what application that the signature refers to).
/// It is used to construct [struct IntentMessage] that what a signature commits to.
///
/// The serialization of an Intent is a 3-byte array where each field is represented by a byte.
#[derive(Serialize)]
pub struct Intent {
    pub scope: IntentScope,
    pub version: IntentVersion,
    pub app_id: AppId,
}

/// Intent Message is a wrapper around a message with its intent. The message can
/// be any type that implements [trait Serialize]. *ALL* signatures in Sui must commits
/// to the intent message, not the message itself. This guarantees any intent
/// message signed in the system cannot collide with another since they are domain
/// separated by intent.
///
/// The serialization of an IntentMessage is compact: it only appends three bytes
/// to the message itself.
#[derive(Serialize)]
pub struct IntentMessage<T> {
    pub intent: Intent,
    pub value: T,
}

pub struct TransactionPreimage {
    /// Transaction `bcs` encoded representation.
    pub unsigned_tx_data: Data,
    /// [`TransactionPreimage::unsigned_tx_data`] extended with the `IntentMessage`.
    pub tx_data_to_sign: Data,
    /// Hash of the [`TransactionPreimage::tx_data_to_sign`].
    pub tx_hash_to_sign: H256,
}

pub struct TxSigner;

impl TxSigner {
    pub fn sign(
        tx: &TransactionData,
        signer_key: &ed25519::sha512::KeyPair,
    ) -> SigningResult<(TransactionPreimage, SuiSignatureInfo)> {
        let public_key = signer_key.public();
        let signer_address = SuiAddress::with_ed25519_pubkey(public_key)?;

        if signer_address != tx.sender() {
            return Err(SigningError(SigningErrorType::Error_missing_private_key));
        }

        let preimage = Self::preimage(tx)?;
        let signature = signer_key.sign(preimage.tx_hash_to_sign.into_vec())?;
        let signature_info = SuiSignatureInfo::ed25519(&signature, public_key);
        Ok((preimage, signature_info))
    }

    pub fn preimage(tx: &TransactionData) -> SigningResult<TransactionPreimage> {
        let unsigned_tx_data =
            bcs::encode(tx).map_err(|_| SigningError(SigningErrorType::Error_invalid_params))?;
        let tx_intent = IntentMessage {
            intent: Intent {
                scope: IntentScope::TransactionData,
                version: IntentVersion::V0,
                app_id: AppId::Sui,
            },
            value: tx,
        };

        let tx_data_to_sign = bcs::encode(&tx_intent)
            .map_err(|_| SigningError(SigningErrorType::Error_invalid_params))?;
        let tx_hash_to_sign = blake2_b(&tx_data_to_sign, H256::LEN)
            .and_then(|hash| H256::try_from(hash.as_slice()))
            .map_err(|_| SigningError(SigningErrorType::Error_internal))?;
        Ok(TransactionPreimage {
            unsigned_tx_data,
            tx_data_to_sign,
            tx_hash_to_sign,
        })
    }
}
