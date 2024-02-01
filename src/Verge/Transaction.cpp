// SPDX-License-Identifier: Apache-2.0
//
// Copyright © 2017 Trust Wallet.

#include "Transaction.h"
#include "../BinaryCoding.h"
#include "../Hash.h"
#include "../Data.h"
#include "../HexCoding.h"

#include "../Bitcoin/SegwitAddress.h"
#include "../Bitcoin/SigHashType.h"
#include "../Bitcoin/SignatureVersion.h"

#include <cassert>

using namespace TW;
namespace TW::Verge {

Data Transaction::getPreImage(const Bitcoin::Script& scriptCode, size_t index,
                              enum TWBitcoinSigHashType hashType, uint64_t amount) const {
    assert(index < inputs.size());

    Data data;

    // Version
    encode32LE(_version, data);

    // Time
    encode32LE(time, data);

    // Input prevouts (none/all, depending on flags)
    if ((hashType & TWBitcoinSigHashTypeAnyoneCanPay) == 0) {
        auto hashPrevouts = getPrevoutHash();
        std::copy(std::begin(hashPrevouts), std::end(hashPrevouts), std::back_inserter(data));
    } else {
        std::fill_n(back_inserter(data), 32, 0);
    }

    // Input nSequence (none/all, depending on flags)
    if ((hashType & TWBitcoinSigHashTypeAnyoneCanPay) == 0 &&
        !Bitcoin::hashTypeIsSingle(hashType) && !Bitcoin::hashTypeIsNone(hashType)) {
        auto hashSequence = getSequenceHash();
        std::copy(std::begin(hashSequence), std::end(hashSequence), std::back_inserter(data));
    } else {
        std::fill_n(back_inserter(data), 32, 0);
    }

    // The input being signed (replacing the scriptSig with scriptCode + amount)
    // The prevout may already be contained in hashPrevout, and the nSequence
    // may already be contain in hashSequence.
    inputs[index].previousOutput.encode(data);
    scriptCode.encode(data);

    encode64LE(amount, data);
    encode32LE(inputs[index].sequence, data);

    // Outputs (none/one/all, depending on flags)
    if (!Bitcoin::hashTypeIsSingle(hashType) && !Bitcoin::hashTypeIsNone(hashType)) {
        auto hashOutputs = getOutputsHash();
        copy(begin(hashOutputs), end(hashOutputs), back_inserter(data));
    } else if (Bitcoin::hashTypeIsSingle(hashType) && index < outputs.size()) {
        Data outputData;
        outputs[index].encode(outputData);
        auto hashOutputs = Hash::hash(hasher, outputData);
        copy(begin(hashOutputs), end(hashOutputs), back_inserter(data));
    } else {
        fill_n(back_inserter(data), 32, 0);
    }

    // Locktime
    encode32LE(lockTime, data);

    // Sighash type
    encode32LE(hashType, data);

    return data;
}

void Transaction::encode(Data& data, enum SegwitFormatMode segwitFormat) const {
    bool useWitnessFormat = true;
    switch (segwitFormat) {
        case NonSegwit: useWitnessFormat = false; break;
        case IfHasWitness: useWitnessFormat = hasWitness(); break;
        case Segwit: useWitnessFormat = true; break;
    }

    encode32LE(_version, data);

    encode32LE(time, data);

    if (useWitnessFormat) {
        // Use extended format in case witnesses are to be serialized.
        data.push_back(0); // marker
        data.push_back(1); // flag
    }

    // txins
    encodeVarInt(inputs.size(), data);
    for (auto& input : inputs) {
        input.encode(data);
    }

    // txouts
    encodeVarInt(outputs.size(), data);
    for (auto& output : outputs) {
        output.encode(data);
    }

    if (useWitnessFormat) {
        encodeWitness(data);
    }

    encode32LE(lockTime, data); // nLockTime
}

Data Transaction::getSignatureHash(const Bitcoin::Script& scriptCode, size_t index,
                                   enum TWBitcoinSigHashType hashType, uint64_t amount,
                                   enum Bitcoin::SignatureVersion version) const {
    switch (version) {
    case Bitcoin::BASE:
        return getSignatureHashBase(scriptCode, index, hashType);
    case Bitcoin::WITNESS_V0:
        return getSignatureHashWitnessV0(scriptCode, index, hashType, amount);
    }
}

/// Generates the signature hash for Witness version 0 scripts.
Data Transaction::getSignatureHashWitnessV0(const Bitcoin::Script& scriptCode, size_t index,
                                            enum TWBitcoinSigHashType hashType,
                                            uint64_t amount) const {
    auto preimage = getPreImage(scriptCode, index, hashType, amount);
    auto hash = Hash::hash(hasher, preimage);
    return hash;
}

/// Generates the signature hash for for scripts other than witness scripts.
Data Transaction::getSignatureHashBase(const Bitcoin::Script& scriptCode, size_t index,
                                       enum TWBitcoinSigHashType hashType) const {
    assert(index < inputs.size());

    Data data;

    encode32LE(_version, data);

    encode32LE(time, data);

    auto serializedInputCount =
        (hashType & TWBitcoinSigHashTypeAnyoneCanPay) != 0 ? 1 : inputs.size();
    encodeVarInt(serializedInputCount, data);
    for (auto subindex = 0ul; subindex < serializedInputCount; subindex += 1) {
        serializeInput(subindex, scriptCode, index, hashType, data);
    }

    auto hashNone = Bitcoin::hashTypeIsNone(hashType);
    auto hashSingle = Bitcoin::hashTypeIsSingle(hashType);
    auto serializedOutputCount = hashNone ? 0 : (hashSingle ? index + 1 : outputs.size());
    encodeVarInt(serializedOutputCount, data);
    for (auto subindex = 0ul; subindex < serializedOutputCount; subindex += 1) {
        if (hashSingle && subindex != index) {
            auto output = Bitcoin::TransactionOutput(-1, {});
            output.encode(data);
        } else {
            outputs[subindex].encode(data);
        }
    }

    // Locktime
    encode32LE(lockTime, data);

    // Sighash type
    encode32LE(hashType, data);

    auto hash = Hash::hash(hasher, data);
    return hash;
}

} // namespace TW::Verge