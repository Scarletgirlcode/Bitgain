// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use serde::Deserialize;

/// Blockchain implementation type.
/// Extend this enum when adding new blockchains.
#[derive(Copy, Clone, Debug, Deserialize, PartialEq)]
pub enum BlockchainType {
    // start_of_blockchain_type - USED TO GENERATE CODE
    Aptos,
    Binance,
    Bitcoin,
    Cosmos,
    Ethereum,
    Greenfield,
    InternetComputer,
    NativeEvmos,
    NativeInjective,
    Ronin,
    Thorchain,
    // end_of_blockchain_type - USED TO GENERATE CODE
    #[serde(other)]
    Unsupported,
}

impl BlockchainType {
    pub fn is_supported(&self) -> bool {
        !matches!(self, BlockchainType::Unsupported)
    }
}
