// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "../Bech32Address.h"
#include "../Data.h"
#include "../PublicKey.h"

#include <string>

namespace TW::Cosmos {

/// A Bech32 Cosmos address.  Hrp has to be specified (e.g. "cosmos", "terra"...)
class Address: public Bech32Address {
public:
    Address() : Bech32Address("") {}

    /// Initializes an address with a key hash.
    Address(std::string hrp, Data keyHash) : Bech32Address(hrp, keyHash) {}

    /// Initializes an address with a public key.
    Address(std::string hrp, const PublicKey& publicKey) : Bech32Address(hrp, HASHER_SHA2_RIPEMD, publicKey) {}

    static bool decode(const std::string& addr, Address& obj_out) {
        return Bech32Address::decode(addr, obj_out, "");
    }
};

} // namespace TW::Cosmos

/// Wrapper for C interface.
struct TWCosmosAddress {
    TW::Cosmos::Address impl;
};
