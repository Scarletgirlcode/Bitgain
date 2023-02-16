// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "Solana/Address.h"
#include "Solana/LegacyMessage.h"
#include "Solana/Hash.h"
#include "Solana/Signature.h"
#include "Data.h"
#include "BinaryCoding.h"

#include <vector>
#include <string>

namespace TW::Solana {

enum TokenAuthorityType {
    MintTokens = 0,
    FreezeAccount = 1,
    AccountOwner = 2,
    CloseAccount = 3,
};

class Transaction {
  public:
    // Signatures
    std::vector<Signature> signatures;
    // The message to sign
    LegacyMessage message;

    Transaction(const LegacyMessage& message) : message(message) {
        this->signatures.resize(message.header.numRequiredSignatures, Signature(defaultSignature));
    }

    // Default basic transfer transaction
    Transaction(const Address& from, const Address& to, uint64_t value, Hash recentBlockhash, std::string memo = "", std::vector<Address> references = {})
        : message(LegacyMessage::createTransfer(from, to, value, recentBlockhash, memo, references)) {
        this->signatures.resize(1, Signature(defaultSignature));
    }

  public:
    std::string serialize() const;
    std::vector<uint8_t> messageData() const;
    uint8_t getAccountIndex(Address publicKey);

  private:
    TW::Data defaultSignature = TW::Data(64);
};

} // namespace TW::Solana
