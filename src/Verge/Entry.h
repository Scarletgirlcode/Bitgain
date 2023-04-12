// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "../CoinEntry.h"

namespace TW::Verge {

/// Entry point for implementation of Verge coin.
/// Note: do not put the implementation here (no matter how simple), to avoid having coin-specific includes in this file
class Entry: public CoinEntry {
public:
    virtual bool validateAddress(TWCoinType coin, const std::string& address, const PrefixVariant& addressPrefix) const;
    virtual std::string deriveAddress(TWCoinType coin, const PublicKey& publicKey, TW::byte p2pkh, const char* hrp) const;
    virtual std::string deriveAddress(TWCoinType coin, TWDerivation derivation, const PublicKey& publicKey, TW::byte p2pkh, const char* hrp) const;
    virtual Data addressToData(TWCoinType coin, const std::string& address) const;
    virtual void sign(TWCoinType coin, const Data& dataIn, Data& dataOut) const;
    virtual void plan(TWCoinType coin, const Data& dataIn, Data& dataOut) const;

    virtual Data preImageHashes(TWCoinType coin, const Data& txInputData) const;
    virtual void compile(TWCoinType coin, const Data& txInputData, const std::vector<Data>& signatures, const std::vector<PublicKey>& publicKeys, Data& dataOut) const;
};

} // namespace TW::Verge
