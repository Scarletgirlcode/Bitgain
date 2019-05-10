// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include <gtest/gtest.h>

#include "ReadData.h"
#include "NEO/CoinReference.hpp"

using namespace std;
using namespace TW;
using namespace TW::NEO;

TEST(NEOCoinReference, Serialize) {
    //  auto a = parse_hex("031bec1250aa8f78275f99a6663688f31085848d0ed92f1203e447125f927b7486");
    auto coinReference = CoinReference();
    coinReference.prevHash = "bdecbb623eee6f9ade28d5a8ff5fb3ea9c9d73af039e0286201b3b0291fb4d4a";
    coinReference.prevIndex = 1;
    auto serialized = coinReference.serialize();
    ASSERT_EQ(parse_hex("bdecbb623eee6f9ade28d5a8ff5fb3ea9c9d73af039e0286201b3b0291fb4d4a1"), serialized);
}

/*
 TEST(NEOAddress, FromString) {
     string neoAddress = "AXkgwcMJTy9wTAXHsbyhauxh7t2Tt31MmC";
     const auto address = Address(neoAddress);
     ASSERT_EQ(address.string(), neoAddress);
 }

TEST(NEOAddress, isValid) {
    string neoAddress = "AQAsqiyHS4SSVWZ4CmMmnCxWg7vJ84GEj4";
    string bitcoinAddress = "1Ma2DrB78K7jmAwaomqZNRMCvgQrNjE2QC";

    ASSERT_TRUE(Address::isValid(neoAddress));
    ASSERT_FALSE(Address::isValid(bitcoinAddress));
}

TEST(NEOAddress, validation) {
    ASSERT_FALSE(Address::isValid("abc"));
    ASSERT_FALSE(Address::isValid("abeb60f3e94c1b9a09f33669435e7ef12eacd"));
    ASSERT_FALSE(Address::isValid("abcb60f3e94c9b9a09f33669435e7ef1beaedads"));
    ASSERT_TRUE(Address::isValid("ANDfjwrUroaVtvBguDtrWKRMyxFwvVwnZD"));
}

TEST(NEOAddress, fromPubKey) {
    auto address = Address(
            PublicKey(parse_hex("031bec1250aa8f78275f99a6663688f31085848d0ed92f1203e447125f927b7486")));
    EXPECT_EQ("AeicEjZyiXKgUeSBbYQHxsU1X3V5Buori5", address.string());
}

TEST(NEOAddress, fromString) {
    auto b58Str = "AYTxeseHT5khTWhtWX1pFFP1mbQrd4q1zz";
    auto address = Address(b58Str);
    EXPECT_EQ(b58Str, address.string());
    auto errB58Str = "AATxeseHT5khTWhtWX1pFFP1mbQrd4q1zz";
    ASSERT_THROW(new Address(errB58Str), std::invalid_argument);
}

TEST(NEOAddress, fromMultiPubKeys) {
    auto signer1 = Signer(PrivateKey(parse_hex("4646464646464646464646464646464646464646464646464646464646464646")));
    auto signer2 = Signer(PrivateKey(parse_hex("4646464646464646464646464646464646464646464646464646464646464652")));
    auto signer3 = Signer(PrivateKey(parse_hex("4646464646464646464646464646464646464646464646464646464646464658")));
    std::vector<Data> pubKeys{signer1.getPublicKey().bytes, signer2.getPublicKey().bytes, signer3.getPublicKey().bytes};
    uint8_t m = 2;
    auto multiAddress = Address(m, pubKeys);
    EXPECT_EQ("AYGWgijVZnrUa2tRoCcydsHUXR1111DgdW", multiAddress.string());
}
*/
