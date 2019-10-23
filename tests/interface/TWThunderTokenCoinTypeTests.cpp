// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeThunderToken) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeThunderToken));
    assertStringsEqual(symbol, "TT");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeThunderToken), 18);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeThunderToken, txId));
    assertStringsEqual(txUrl, "https://scan.thundercore.com/transactions/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeThunderToken));
    assertStringsEqual(id, "thundertoken");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeThunderToken));
    assertStringsEqual(name, "Thunder Token");

    ASSERT_EQ(TWBlockchainEthereum, TWCoinTypeBlockchain(TWCoinTypeThunderToken));
}

