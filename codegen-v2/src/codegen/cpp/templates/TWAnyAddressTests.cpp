// Copyright © 2017-{YEAR} Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include <TrustWalletCore/TWAnyAddress.h>
#include "HexCoding.h"

#include "TestUtilities.h"
#include <gtest/gtest.h>

using namespace TW;

// TODO: Finalize tests

TEST(TW{COIN_TYPE}, Address) {
    // TODO: Finalize test implementation

    auto string = STRING("__ADD_VALID_ADDRESS_HERE__");
    auto addr = WRAP(TWAnyAddress, TWAnyAddressCreateWithString(string.get(), TWCoinType{COIN_TYPE}));
    auto string2 = WRAPS(TWAnyAddressDescription(addr.get()));
    EXPECT_TRUE(TWStringEqual(string.get(), string2.get()));
    auto keyHash = WRAPD(TWAnyAddressData(addr.get()));
    assertHexEqual(keyHash, "__CORRESPONDING_ADDRESS_DATA__");
}
