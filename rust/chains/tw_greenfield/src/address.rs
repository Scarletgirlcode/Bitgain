// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use serde::Serialize;
use std::fmt;
use std::str::FromStr;
use tw_coin_entry::coin_context::CoinContext;
use tw_coin_entry::coin_entry::CoinAddress;
use tw_coin_entry::error::{AddressError, AddressResult};
use tw_cosmos_sdk::address::CosmosAddress;
use tw_evm::address::Address as EthereumAddress;
use tw_keypair::ecdsa::secp256k1;
use tw_memory::Data;

#[derive(Clone, Serialize)]
pub struct GreenfieldAddress(EthereumAddress);

impl GreenfieldAddress {
    /// Initializes an address with a `secp256k1` public key.
    pub fn with_secp256k1_pubkey(pubkey: &secp256k1::PublicKey) -> GreenfieldAddress {
        GreenfieldAddress(EthereumAddress::with_secp256k1_pubkey(pubkey))
    }
}

impl CosmosAddress for GreenfieldAddress {
    fn from_str_with_coin(_coin: &dyn CoinContext, addr: &str) -> AddressResult<Self>
    where
        Self: Sized,
    {
        GreenfieldAddress::from_str(addr)
    }
}

impl CoinAddress for GreenfieldAddress {
    #[inline]
    fn data(&self) -> Data {
        self.0.data()
    }
}

impl FromStr for GreenfieldAddress {
    type Err = AddressError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        EthereumAddress::from_str(s).map(GreenfieldAddress)
    }
}

impl fmt::Display for GreenfieldAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}
