// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeRavencoin) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeRavencoin));
    assertStringsEqual(symbol, "RVN");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeRavencoin), 8);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeRavencoin, txId));
    assertStringsEqual(txUrl, "https://ravencoin.network/tx/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeRavencoin));
    assertStringsEqual(id, "ravencoin");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeRavencoin));
    assertStringsEqual(name, "Ravencoin");

    ASSERT_EQ(TWBlockchainBitcoin, TWCoinTypeBlockchain(TWCoinTypeRavencoin));
    ASSERT_EQ(0x7a, TWCoinTypeP2shPrefix(TWCoinTypeRavencoin));
}

