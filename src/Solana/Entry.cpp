// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Entry.h"

#include "Address.h"
#include "Signer.h"

using namespace TW;
using namespace std;

namespace TW::Solana {

// Note: avoid business logic from here, rather just call into classes like Address, Signer, etc.

bool Entry::validateAddress([[maybe_unused]] TWCoinType coin, const string& address, TW::byte, TW::byte, const char*) const {
    return Address::isValid(address);
}

string Entry::deriveAddress([[maybe_unused]] TWCoinType coin, const PublicKey& publicKey, TW::byte, const char*) const {
    return Address(publicKey).string();
}

Data Entry::addressToData([[maybe_unused]] TWCoinType coin, const std::string& address) const {
    return Address(address).vector();
}

void Entry::sign([[maybe_unused]] TWCoinType coin, const TW::Data& dataIn, TW::Data& dataOut) const {
    signTemplate<Signer, Proto::SigningInput>(dataIn, dataOut);
}

string Entry::signJSON([[maybe_unused]] TWCoinType coin, const std::string& json, const Data& key) const {
    return Signer::signJSON(json, key);
}

TW::Data Entry::preImageHashes([[maybe_unused]] TWCoinType coin, const TW::Data& txInputData) const {
    return txCompilerTemplate<Proto::SigningInput, Proto::PreSigningOutput>(
        txInputData, [](const auto& input, auto& output) {
            auto signer = Signer(input);
            auto preimageHash = signer.preImageHash();
            // for Solana, there is no need to hash data.
            output.set_data(preimageHash.data(), preimageHash.size());
            auto signers = signer.signers();
            auto nSigners = output.mutable_signers();
            for (auto i = 0ul; i < signers.size();i++) {
                auto newSigner = nSigners->Add();
                *newSigner = signers[i];
            }
        });
}

void Entry::compile([[maybe_unused]] TWCoinType coin, const Data& txInputData, const std::vector<Data>& signatures,
                    const std::vector<PublicKey>& publicKeys, Data& dataOut) const {
    dataOut = txCompilerTemplate<Proto::SigningInput, Proto::SigningOutput>(
        txInputData, [&](const auto& input, auto& output) {
            auto signer = Signer(input);
            output = signer.compile(signatures, publicKeys);
        });
}

} // namespace TW::Solana
