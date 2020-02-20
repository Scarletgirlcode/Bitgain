// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Ethereum/EIP712/Encoder.h"
#include <HexCoding.h>

#include <gtest/gtest.h>

using namespace TW;
using namespace TW::Ethereum;

TEST(EthereumEIP712, encodeBool) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeBool(false)), "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeBool(true)), "0000000000000000000000000000000000000000000000000000000000000001");
}

TEST(EthereumEIP712, encodeInt) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt32(69)), "0000000000000000000000000000000000000000000000000000000000000045");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt32(-1)), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt32(0)), "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt32(1)), "0000000000000000000000000000000000000000000000000000000000000001");

    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt32(69)), "0000000000000000000000000000000000000000000000000000000000000045");
    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt32(0)), "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt32(1)), "0000000000000000000000000000000000000000000000000000000000000001");

    EXPECT_EQ(hex(EIP712::Encoder::encodeInt256(69)), "0000000000000000000000000000000000000000000000000000000000000045");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt256(-1)), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt256(0)), "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeInt256(1)), "0000000000000000000000000000000000000000000000000000000000000001");

    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt256(69)), "0000000000000000000000000000000000000000000000000000000000000045");
    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt256(0)), "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeUInt256(1)), "0000000000000000000000000000000000000000000000000000000000000001");
}

TEST(EthereumEIP712, encodeAddress) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeAddress(parse_hex("0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed"))), "0000000000000000000000005aaeb6053f3e94c9b9a09f33669435e7ef1beaed");
}

TEST(EthereumEIP712, encodeString) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeString("trustwallet")), "31924c4e2bb082322d1efa718bf67c73ca297b481dac9f76ad35670cff0056a3");
}

TEST(EthereumEIP712, encodeBytes) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeBytes(parse_hex("45"))), "4500000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeBytes(parse_hex("5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed"))), "5aaeb6053f3e94c9b9a09f33669435e7ef1beaed000000000000000000000000");
    EXPECT_EQ(hex(EIP712::Encoder::encodeBytes(parse_hex("000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f"))), "000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f");
    // too long, truncated
    EXPECT_EQ(hex(EIP712::Encoder::encodeBytes(parse_hex("000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f3333"))), "000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f");
}

TEST(EthereumEIP712, encodeBytesDyn) {
    EXPECT_EQ(hex(EIP712::Encoder::encodeBytesDyn(TW::data(std::string("trustwallet")))), "31924c4e2bb082322d1efa718bf67c73ca297b481dac9f76ad35670cff0056a3");
}

TEST(EthereumEIP712, uint256FromInt256) {
    EXPECT_EQ(EIP712::Encoder::uint256FromInt256(0), 0);
    EXPECT_EQ(EIP712::Encoder::uint256FromInt256(1), 1);
    EXPECT_EQ(EIP712::Encoder::uint256FromInt256(100), 100);
    EXPECT_EQ(EIP712::Encoder::uint256FromInt256(-1), (~uint256_t(0)));
    EXPECT_EQ(EIP712::Encoder::uint256FromInt256(-2), (~uint256_t(1)));
}
