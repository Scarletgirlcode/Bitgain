// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::address::GreenfieldAddress;
use crate::public_key::GreenfieldPublicKey;
use tw_cosmos_sdk::context::CosmosContext;
use tw_cosmos_sdk::hasher::keccak256_hasher::Keccak256Hasher;
use tw_cosmos_sdk::private_key::secp256k1::Secp256PrivateKey;
use tw_cosmos_sdk::signature::secp256k1::Secp256k1Signature;

pub struct GreenfieldContext;

impl CosmosContext for GreenfieldContext {
    type Address = GreenfieldAddress;
    /// Greenfield uses EIP712 message signing algorithm built upon `keccak256` hash.
    type TxHasher = Keccak256Hasher;
    type PrivateKey = Secp256PrivateKey;
    type PublicKey = GreenfieldPublicKey;
    type Signature = Secp256k1Signature;
}
