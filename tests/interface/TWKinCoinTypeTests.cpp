// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeKin) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeKin));
    assertStringsEqual(symbol, "KIN");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeKin), 5);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeKin, txId));
    assertStringsEqual(txUrl, "https://kinexplorer.com/tx/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeKin));
    assertStringsEqual(id, "kin");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeKin));
    assertStringsEqual(name, "Kin");

    ASSERT_EQ(TWBlockchainStellar, TWCoinTypeBlockchain(TWCoinTypeKin));
}

