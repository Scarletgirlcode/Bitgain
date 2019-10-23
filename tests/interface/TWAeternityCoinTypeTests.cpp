// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeAeternity) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeAeternity));
    assertStringsEqual(symbol, "AE");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeAeternity), 18);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeAeternity, txId));
    assertStringsEqual(txUrl, "https://explorer.aepps.com/#/tx/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeAeternity));
    assertStringsEqual(id, "aeternity");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeAeternity));
    assertStringsEqual(name, "Aeternity");

    ASSERT_EQ(TWBlockchainAeternity, TWCoinTypeBlockchain(TWCoinTypeAeternity));
}

