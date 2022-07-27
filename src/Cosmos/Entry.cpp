// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Entry.h"

#include "Address.h"
#include "Signer.h"

//using namespace TW::Cosmos;
using namespace TW;
using namespace std;

// Note: avoid business logic from here, rather just call into classes like Address, Signer, etc.

bool Cosmos::Entry::validateAddress(TWCoinType coin, const string& address, TW::byte, TW::byte, [[maybe_unused]] const char* hrp) const {
    return Address::isValid(coin, address);
}

string Cosmos::Entry::deriveAddress(TWCoinType coin, const PublicKey& publicKey, TW::byte, [[maybe_unused]] const char* hrp) const {
    return Address(coin, publicKey).string();
}

Data Cosmos::Entry::addressToData([[maybe_unused]] TWCoinType coin, const std::string& address) const {
    Address addr;
    if (!Address::decode(address, addr)) {
        return Data();
    }
    return addr.getKeyHash();
}

void Cosmos::Entry::sign(TWCoinType coin, const TW::Data& dataIn, TW::Data& dataOut) const {
    auto input = Proto::SigningInput();
    input.ParseFromArray(dataIn.data(), (int)dataIn.size());
    auto serializedOut = Signer::sign(input, coin).SerializeAsString();
    dataOut.insert(dataOut.end(), serializedOut.begin(), serializedOut.end());
}

string Cosmos::Entry::signJSON(TWCoinType coin, const std::string& json, const Data& key) const {
    return Signer::signJSON(json, key, coin);
}
