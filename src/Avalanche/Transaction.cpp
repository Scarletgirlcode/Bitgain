// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Transaction.h"

#include "../BinaryCoding.h"

using namespace TW::Avalanche;

void BaseTransaction::baseEncode(Data& data) const {
    encode32LE(TypeID, data);
    encode32LE(NetworkID, data);
    for (auto byte : BlockchainID) {
        data.push_back(byte);
    }
    std::sort(Outputs.begin(), Outputs.end());
    encode32LE(Outputs.size(), data);
    for (auto output : Outputs) {
        output.encode(data);
    }
    std::sort(Inputs.begin(), Inputs.end());
    encode32LE(Inputs.size(), data);
    for (auto input : Inputs) {
        input.encode(data);
    }
    for (auto byte : Memo) {
        data.push_back(byte);
    }
}

void UnsignedCreateAssetTransaction::encode(Data& data) const {
    baseEncode(data);
    encodeString(Name, data);
    encodeString(Symbol, data);
    data.push_back(Denomination);
    std::sort(InitialStates.begin(), InitialStates.end());
    encode32LE(InitialStates.size(), data);
    for (auto state : InitialStates) {
        state.encode(data);
    }
}

void UnsignedOperationTransaction::encode(Data& data) const {
    baseEncode(data);
    std::sort(Operations.begin(), Operations.end());
    encode32LE(Operations.size(), data);
    for (auto op : Operations) {
        op.encode(data);
    }
}

void UnsignedImportTransaction::encode(Data& data) const {
    baseEncode(data);
    for (auto byte : SourceChain) {
        data.push_back(byte);
    }
    std::sort(ImportInputs.begin(), ImportInputs.end());
    encode32LE(ImportInputs.size(), data);
    for (auto in : ImportInputs) {
        in.encode(data);
    }
}


void UnsignedExportTransaction::encode(Data& data) const {
    baseEncode(data);
    for (auto byte : DestinationChain) {
        data.push_back(byte);
    }
    std::sort(ExportOutputs.begin(), ExportOutputs.end());
    encode32LE(ExportOutputs.size(), data);
    for (auto out : ExportOutputs) {
        out.encode(data);
    }
}