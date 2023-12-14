// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::transaction::UnsignedTransaction;
use tw_coin_entry::error::{SigningError, SigningErrorType, SigningResult};
use tw_hash::{sha2, H256};

pub struct JsonTxPreimage {
    pub encoded_tx: String,
    pub tx_hash: H256,
}

pub struct JsonPreimager;

impl JsonPreimager {
    pub fn preimage_hash(unsigned: &UnsignedTransaction) -> SigningResult<JsonTxPreimage> {
        let encoded_tx = serde_json::to_string(unsigned)
            .map_err(|_| SigningError(SigningErrorType::Error_internal))?;
        let tx_hash = sha2::sha256(encoded_tx.as_bytes());
        let tx_hash = H256::try_from(tx_hash.as_slice()).expect("sha256 must return 32 bytes");
        Ok(JsonTxPreimage {
            encoded_tx,
            tx_hash,
        })
    }
}
