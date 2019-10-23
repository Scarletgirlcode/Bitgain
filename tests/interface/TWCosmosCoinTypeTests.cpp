// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "TWTestUtilities.h"

#include <TrustWalletCore/TWCoinTypeConfiguration.h>

#include <gtest/gtest.h>


// Originally one-time generated by script gen_test_cointype.py

TEST(TWCoinTypeConfiguration, TWCoinTypeCosmos) {
    auto symbol = WRAPS(TWCoinTypeConfigurationGetSymbol(TWCoinTypeCosmos));
    assertStringsEqual(symbol, "ATOM");

    ASSERT_EQ(TWCoinTypeConfigurationGetDecimals(TWCoinTypeCosmos), 6);

    auto txId = TWStringCreateWithUTF8Bytes("123");
    auto txUrl = WRAPS(TWCoinTypeConfigurationGetTransactionURL(TWCoinTypeCosmos, txId));
    assertStringsEqual(txUrl, "https://www.mintscan.io/txs/123");

    auto id = WRAPS(TWCoinTypeConfigurationGetID(TWCoinTypeCosmos));
    assertStringsEqual(id, "cosmos");

    auto name = WRAPS(TWCoinTypeConfigurationGetName(TWCoinTypeCosmos));
    assertStringsEqual(name, "Cosmos");

    ASSERT_EQ(TWBlockchainCosmos, TWCoinTypeBlockchain(TWCoinTypeCosmos));
}

