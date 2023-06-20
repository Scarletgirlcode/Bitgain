// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "Amount.h"
#include "Script.h"
#include "Data.h"
#include "PublicKey.h"
#include "proto/Bitcoin.pb.h"

#include <memory>

namespace TW::Bitcoin {

/// Bitcoin transaction output.
struct TransactionOutput {
    /// Transaction amount.
    Amount value;

    /// Usually contains the public key as a Bitcoin script setting up
    /// conditions to claim this output.
    Script script;

    /// Initializes an empty transaction output.
    TransactionOutput() = default;

    /// Initializes a transaction output with a value and a script.
    TransactionOutput(Amount value, Script script) : value(value), script(std::move(script)) {}

    /// Encodes the output into the provided buffer.
    void encode(Data& data) const;
};

TW::Bitcoin::Proto::TransactionOutput buildP2wpkhScript(Amount value, const TW::PublicKey& publicKey);
TW::Bitcoin::Proto::TransactionOutput buildBRC20InscribeTransfer(const std::string& ticker, uint64_t brcAmount, Amount brcInscribeAmount, const TW::PublicKey& publicKey);


} // namespace TW::Bitcoin
