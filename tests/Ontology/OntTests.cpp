// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "HexCoding.h"

#include "Ontology/Ont.h"

#include <string>
#include <iostream>
#include <unordered_map>

#include <gtest/gtest.h>
#include <boost/any.hpp>

using namespace TW;
using namespace TW::Ontology;


TEST(OntologyOnt, queryBalance) {
    auto address = Address("ANDfjwrUroaVtvBguDtrWKRMyxFwvVwnZD");
    auto tx = Ont().balanceOf(address);
    auto serializedTx = hex(tx.serialize());
    EXPECT_EQ(0, serializedTx.find("00d1"));
    ASSERT_EQ(86, serializedTx.find("1446b1a18af6b7c9f8a4602f9f73eeb3030f0c29b70962616c616e63654f66140000000000000000000000000000000000000001"));
}

TEST(OntologyOnt, transfer) {
    std::vector<uint8_t> ontContract{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    auto acct1 = Account("4646464646464646464646464646464646464646464646464646464646464646");
    auto acct2 = Account("4646464646464646464646464646464646464646464646464646464646464652");
    auto fromAddress = Address("ANDfjwrUroaVtvBguDtrWKRMyxFwvVwnZD");
    std::string toAddress = "Af1n2cZHhMZumNqKgw9sfCNoTWu9de4NDn";
    uint64_t amount = 1;
    uint64_t gasPrice = 500;
    uint64_t gasLimit = 20000;
    auto tx = Ont().transfer(acct1, toAddress, amount, acct2, gasPrice, gasLimit);
    auto serializedTx = hex(tx.serialize());
    EXPECT_EQ(724, serializedTx.length());
    EXPECT_EQ(0, serializedTx.find("00d1"));
    EXPECT_EQ(13, serializedTx.find("401000000000000204e000000000000"));
    EXPECT_EQ(86, serializedTx.find("00c66b14fbacc8214765d457c8e3f2b5a1d3c4981a2e9d2a6a7cc814feec06b79ed299ea06fcb94abac41aaf3ead76586a7cc8516a7cc86c51c1087472616e73666572"));
    EXPECT_EQ(220, serializedTx.find("1400000000000000000000000000000000000000010068164f6e746f6c6f67792e4e61746976652e496e766f6b"));
    EXPECT_EQ(452, serializedTx.find("031bec1250aa8f78275f99a6663688f31085848d0ed92f1203e447125f927b7486"));
    EXPECT_EQ(656, serializedTx.find("03d9fd62df332403d9114f3fa3da0d5aec9dfa42948c2f50738d52470469a1a1ee"));
}