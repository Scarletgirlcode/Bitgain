// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use tw_coin_entry::error::SigningResult;
use tw_coin_registry::coin_type::CoinType;
use tw_coin_registry::dispatcher::coin_dispatcher;
use tw_memory::Data;

/// Represents a message signer to sign regular or typed structured data for any blockchain.
pub struct MessageSigner;

impl MessageSigner {
    /// Signs a message.
    #[inline]
    pub fn sign_message(input: &[u8], coin: CoinType) -> SigningResult<Data> {
        let (ctx, entry) = coin_dispatcher(coin)?;
        entry.sign_message(&ctx, input)
    }

    /// Verifies a signature for a message.
    #[inline]
    pub fn verify_message(input: &[u8], coin: CoinType) -> SigningResult<bool> {
        let (ctx, entry) = coin_dispatcher(coin)?;
        entry.verify_message(&ctx, input)
    }
}
