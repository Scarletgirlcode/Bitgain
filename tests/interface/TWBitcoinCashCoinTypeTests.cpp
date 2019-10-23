// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeBitcoinCash) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeBitcoinCash));
    assertStringsEqual(symbol, "BCH");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeBitcoinCash), 8);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeBitcoinCash, txId));
    assertStringsEqual(txUrl, "https://blockchair.com/bitcoin-cash/transaction/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeBitcoinCash));
    assertStringsEqual(id, "bitcoincash");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeBitcoinCash));
    assertStringsEqual(name, "Bitcoin Cash");

    ASSERT_EQ(TWBlockchainBitcoin, TWCoinTypeBlockchain(TWCoinTypeBitcoinCash));
    ASSERT_EQ(0x05, TWCoinTypeP2shPrefix(TWCoinTypeBitcoinCash));
}

