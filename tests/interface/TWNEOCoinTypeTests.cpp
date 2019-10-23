// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeNEO) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeNEO));
    assertStringsEqual(symbol, "NEO");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeNEO), 8);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeNEO, txId));
    assertStringsEqual(txUrl, "https://neoscan.io/transaction/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeNEO));
    assertStringsEqual(id, "neo");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeNEO));
    assertStringsEqual(name, "NEO");

    ASSERT_EQ(TWBlockchainNEO, TWCoinTypeBlockchain(TWCoinTypeNEO));
}

