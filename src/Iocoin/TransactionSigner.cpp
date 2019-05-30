// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TransactionSigner.h"

#include "TransactionInput.h"
//XXXX #include "TransactionOutput.h"
#include "UnspentSelector.h"

#include "../BinaryCoding.h"
#include "../Hash.h"
#include "../HexCoding.h"

using namespace TW;
using namespace TW::Iocoin;

template <typename Transaction>
Result<Transaction> TransactionSigner<Transaction>::sign() {
    signedInputs.clear();
    std::copy(std::begin(transaction.inputs), std::end(transaction.inputs),
              std::back_inserter(signedInputs));

    const bool hashSingle =
        ((input.hash_type() & ~TWSignatureHashTypeAnyoneCanPay) == TWSignatureHashTypeSingle);
    for (auto i = 0; i < plan.utxos.size(); i += 1) {
        auto& utxo = plan.utxos[i];

        // Only sign TWSignatureHashTypeSingle if there's a corresponding output
        if (hashSingle && i >= transaction.outputs.size()) {
            continue;
        }
        auto script = Bitcoin::Script(utxo.script().begin(), utxo.script().end());
        auto result = sign(script, i, utxo);
        if (!result) {
            return Result<Transaction>::failure(result.error());
        }
    }

    Transaction tx(transaction);
    tx.inputs = move(signedInputs);
    tx.outputs = transaction.outputs;
    return Result<Transaction>::success(std::move(tx));
}

template <typename Transaction>
Result<void> TransactionSigner<Transaction>::sign(Bitcoin::Script script, size_t index,
                                                  const Iocoin::Proto::UnspentTransaction& utxo) {
	Bitcoin::Script redeemScript;
    std::vector<Data> results;

    auto result = signStep(script, index, utxo);
    if (result) {
        results = result.payload();
    } else {
        return Result<void>::failure(result.error());
    }
    auto txin = transaction.inputs[index];

    if (script.isPayToScriptHash()) {
        script = Bitcoin::Script(results.front().begin(), results.front().end());
        auto result = signStep(script, index, utxo);
        if (!result) {
            return Result<void>::failure(result.error());
        }
        results = result.payload();
        results.push_back(script.bytes);
        redeemScript = script;
    }

    if (!redeemScript.bytes.empty()) {
        results.push_back(redeemScript.bytes);
    }

    signedInputs[index] =
        TransactionInput(txin.previousOutput, Bitcoin::Script(pushAll(results)));
    return Result<void>::success();
}

template <typename Transaction>
Result<std::vector<Data>> TransactionSigner<Transaction>::signStep(
    Bitcoin::Script script, size_t index, const Iocoin::Proto::UnspentTransaction& utxo) {
    Transaction transactionToSign(transaction);
    transactionToSign.inputs = signedInputs;
    transactionToSign.outputs = transaction.outputs;

    Data data;
    std::vector<Data> keys;
    int required;

    if (script.matchPayToScriptHash(data)) {
        auto redeemScript = scriptForScriptHash(data);
        if (redeemScript.empty()) {
            // Error: Missing redeem script
            return Result<std::vector<Data>>::failure("Missing redeem script.");
        }
        return Result<std::vector<Data>>::success({redeemScript});
    } else if (script.matchMultisig(keys, required)) {
        auto results = std::vector<Data>{{}}; // workaround CHECKMULTISIG bug
        for (auto& pubKey : keys) {
            if (results.size() >= required + 1) {
                break;
            }
            auto keyHash = TW::Hash::ripemd(TW::Hash::sha256(pubKey));
            auto key = keyForPublicKeyHash(keyHash);
            if (key.empty()) {
                // Error: missing key
                return Result<std::vector<Data>>::failure("Missing private key.");
            }
            auto signature =
                createSignature(transactionToSign, script, key, index, utxo.amount());
            if (signature.empty()) {
                // Error: Failed to sign
                return Result<std::vector<Data>>::failure("Failed to sign.");
            }
            results.push_back(signature);
        }
        results.resize(required + 1);
        return Result<std::vector<Data>>::success(std::move(results));
    } else if (script.matchPayToPubkey(data)) {
        auto keyHash = TW::Hash::ripemd(TW::Hash::sha256(data));
        auto key = keyForPublicKeyHash(keyHash);
        if (key.empty()) {
            // Error: Missing key
            return Result<std::vector<Data>>::failure("Missing private key.");
        }
        auto signature =
            createSignature(transactionToSign, script, key, index, utxo.amount());
        if (signature.empty()) {
            // Error: Failed to sign
            return Result<std::vector<Data>>::failure("Failed to sign.");
        }
        return Result<std::vector<Data>>::success({signature});
    } else if (script.matchPayToPubkeyHash(data)) {
        auto key = keyForPublicKeyHash(data);
        if (key.empty()) {
            // Error: Missing keyxs
            return Result<std::vector<Data>>::failure("Missing private key.");
        }

        auto pubkey = PrivateKey(key).getPublicKey(TWPublicKeyTypeSECP256k1);
        auto signature =
            createSignature(transactionToSign, script, key, index, utxo.amount());
        if (signature.empty()) {
            // Error: Failed to sign
            return Result<std::vector<Data>>::failure("Failed to sign.");
        }
        return Result<std::vector<Data>>::success({signature, pubkey.bytes});
    } else {
        // Error: Invalid output script
        return Result<std::vector<Data>>::failure("Invalid output script.");
    }
}

template <typename Transaction>
Data TransactionSigner<Transaction>::createSignature(const Transaction& transaction,
                                                     const Bitcoin::Script& script, const Data& key,
                                                     size_t index, Bitcoin::Amount amount
                                                     ) {
    auto sighash = transaction.getSignatureHash(script, index, input.hash_type()
                                                );
    s_h.hashSize = sighash.size();
    s_h.hashStr = hex(sighash.begin(), sighash.end());
    auto pk = PrivateKey(key);
    auto sig = pk.signAsDER(Data(begin(sighash), end(sighash)), TWCurveSECP256k1);
    if (sig.empty()) {
        return {};
    }
    sig.push_back(static_cast<uint8_t>(input.hash_type()));
    s_h.sig = sig;
    return sig;
}

template <typename Transaction>
Data TransactionSigner<Transaction>::pushAll(const std::vector<Data>& results) {
    auto data = Data{};
    for (auto& result : results) {
        if (result.empty()) {
            data.push_back(OP_0);
        } else if (result.size() == 1 && result[0] >= 1 && result[0] <= 16) {
            data.push_back(Bitcoin::Script::encodeNumber(result[0]));
        } else if (result.size() < OP_PUSHDATA1) {
            data.push_back(static_cast<uint8_t>(result.size()));
        } else if (result.size() <= 0xff) {
            data.push_back(OP_PUSHDATA1);
            data.push_back(static_cast<uint8_t>(result.size()));
        } else if (result.size() <= 0xffff) {
            data.push_back(OP_PUSHDATA2);
            encode16LE(static_cast<uint16_t>(result.size()), data);
        } else {
            data.push_back(OP_PUSHDATA4);
            encode32LE(static_cast<uint32_t>(result.size()), data);
        }
        std::copy(begin(result), end(result), back_inserter(data));
    }
    return data;
}

template <typename Transaction>
Data TransactionSigner<Transaction>::keyForPublicKeyHash(const Data& hash) const {
    for (auto& key : input.private_key()) {
        auto publicKey = PrivateKey(key).getPublicKey(TWPublicKeyTypeSECP256k1);
        auto keyHash = TW::Hash::ripemd(TW::Hash::sha256(publicKey.bytes));
        if (std::equal(std::begin(keyHash), std::end(keyHash), std::begin(hash), std::end(hash))) {
            return Data(key.begin(), key.end());
        }
    }
    return {};
}

template <typename Transaction>
Data TransactionSigner<Transaction>::scriptForScriptHash(const Data& hash) const {
    auto hashString = hex(hash.begin(), hash.end());
    auto it = input.scripts().find(hashString);
    if (it == input.scripts().end()) {
        // Error: Missing redeem script
        return {};
    }
    return Data(it->second.begin(), it->second.end());
}

// Explicitly instantiate a Signers for compatible transactions.
template class TW::Iocoin::TransactionSigner<Iocoin::Transaction>;
