// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Ont.h"
#include "ParamsBuilder.h"

#include <TrezorCrypto/rand.h>

#include <unordered_map>

using namespace TW;
using namespace TW::Ontology;

Transaction Ont::decimals(const Address& address) {
    auto builder = ParamsBuilder();
    auto invokeCode =
        ParamsBuilder::buildNativeInvokeCode(contractAddress(), version, "decimals", address.data);
    auto tx = Transaction((uint8_t)0, txType, random32(), (uint64_t)0, (uint64_t)0,
                          (std::string) "", invokeCode);
    return tx;
}

Transaction Ont::balanceOf(const Address& address) {
    auto builder = ParamsBuilder();
    auto invokeCode =
        ParamsBuilder::buildNativeInvokeCode(contractAddress(), version, "balanceOf", address.data);
    auto tx = Transaction((uint8_t)0, txType, random32(), (uint64_t)0, (uint64_t)0,
                          (std::string) "", invokeCode);
    return tx;
}

Transaction Ont::transfer(const Signer& from, const Address& to, uint64_t amount,
                          const Signer& payer, uint64_t gasPrice, uint64_t gasLimit) {
    std::list<boost::any> transferParam{from.getAddress().data, to.data, amount};
    std::vector<boost::any> args{transferParam};
    auto invokeCode =
        ParamsBuilder::buildNativeInvokeCode(contractAddress(), 0x00, "transfer", args);
    auto tx = Transaction(version, txType, random32(), gasPrice, gasLimit,
                          payer.getAddress().string(), invokeCode);
    from.sign(tx);
    payer.addSign(tx);
    return tx;
}
