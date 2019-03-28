// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include <string>

#include "Transaction.h"
#include "../PrivateKey.h"
#include "../Data.h"

namespace TW {
namespace Theta {

/// Helper class that performs Theta transaction signing
class Signer {
public:
    std::string chainID;

    Signer() = default;
    /// Initializes a signer with a chain identifier which could be `mainnet`, `testnet` or `privatenet`
    explicit Signer(const std::string& chainID) : chainID(chainID) {}

    /// Signs the given transaction
    Data sign(const PrivateKey& privateKey, const Transaction& transaction) noexcept;

private:
    Data dataForSign(const Transaction& transaction) noexcept;
};

}} // namespace

/// Wrapper for C interface.
struct TWThetaSigner {
    TW::Theta::Signer impl;
};
