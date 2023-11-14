// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Entry.h"

#include "Cosmos/Entry.h"

#include "proto/Cosmos.pb.h"

namespace TW::NativeEvmos {

// Note: avoid business logic from here, rather just call into classes like Address, Signer, etc.

bool Entry::validateAddress(TWCoinType coin, const std::string& address, [[maybe_unused]] const PrefixVariant& addressPrefix) const {
    return validateAddressRust(coin, address, addressPrefix);
}

std::string Entry::deriveAddress(TWCoinType coin, const PublicKey& publicKey, [[maybe_unused]] TWDerivation derivation, [[maybe_unused]] const PrefixVariant& addressPrefix) const {
    return deriveAddressRust(coin, publicKey, derivation, addressPrefix);
}

Data Entry::addressToData(TWCoinType coin, const std::string& address) const {
    return addressToDataRust(coin, address);
}

void Entry::sign(TWCoinType coin, const TW::Data& dataIn, TW::Data& dataOut) const {
    signRust(dataIn, coin, dataOut);
}

// TODO call `signRustJSON` when it's done.
std::string Entry::signJSON(TWCoinType coin, const std::string& json, const Data& key) const {
    return Cosmos::Entry().signJSON(coin, json, key);
}

TW::Data Entry::preImageHashes(TWCoinType coin, const Data& txInputData) const {
    return preImageHashesRust(coin, txInputData);
}

void Entry::compile(TWCoinType coin, const Data& txInputData, const std::vector<Data>& signatures, const std::vector<PublicKey>& publicKeys, Data& dataOut) const {
    compileRust(coin, txInputData, signatures, publicKeys, dataOut);
}

} // namespace TW::NativeEvmos
