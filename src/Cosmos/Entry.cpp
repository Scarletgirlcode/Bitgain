// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Entry.h"
#include "Address.h"
#include "Signer.h"

#include <proto/TransactionCompiler.pb.h>
#include <google/protobuf/util/json_util.h>

using namespace TW;
using namespace std;

namespace TW::Cosmos {

bool Entry::validateAddress(TWCoinType coin, const std::string& address, const PrefixVariant& addressPrefix) const {
    return validateAddressRust(coin, address, addressPrefix);
}

std::string Entry::deriveAddress(TWCoinType coin, const PublicKey& publicKey, [[maybe_unused]] TWDerivation derivation, const PrefixVariant& addressPrefix) const {
    return deriveAddressRust(coin, publicKey, derivation, addressPrefix);
}

Data Entry::addressToData([[maybe_unused]] TWCoinType coin, const std::string& address) const {
    return addressToDataRust(coin, address);
}

void Entry::sign(TWCoinType coin, const TW::Data& dataIn, TW::Data& dataOut) const {
    signRust(dataIn, coin, dataOut);
}

// TODO call `signRustJSON` when it's done.
string Entry::signJSON(TWCoinType coin, const std::string& json, const Data& key) const {
    auto input = Proto::SigningInput();
    google::protobuf::util::JsonStringToMessage(json, &input);
    input.set_private_key(key.data(), key.size());

    auto inputData = data(input.SerializeAsString());
    Data dataOut;
    sign(coin, inputData, dataOut);

    if (dataOut.empty()) {
        return {};
    }

    Proto::SigningOutput output;
    output.ParseFromArray(dataOut.data(), static_cast<int>(dataOut.size()));

    return output.json();
}

Data Entry::preImageHashes(TWCoinType coin, const Data& txInputData) const {
    return preImageHashesRust(coin, txInputData);
}

void Entry::compile(TWCoinType coin, const Data& txInputData, const std::vector<Data>& signatures, const std::vector<PublicKey>& publicKeys, Data& dataOut) const {
    compileRust(coin, txInputData, signatures, publicKeys, dataOut);
}

} // namespace TW::Cosmos
