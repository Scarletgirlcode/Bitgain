// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "ImmutableX/StarkKey.h"
#include "Ethereum/Signer.h"
#include <gtest/gtest.h>

namespace TW::ImmutableX::tests {

TEST(ImmutableX, PathFromAddress) {
    // https://github.com/immutable/imx-core-sdk-swift/blob/main/Tests/ImmutableXCoreTests/Crypto/Stark/StarkKeyTests.swift#L30
    auto res = accountPathFromAddress("0xa76e3eeb2f7143165618ab8feaabcd395b6fac7f");
    ASSERT_EQ(res, "m/2645'/579218131'/211006541'/1534045311'/1431804530'/1");
}

TEST(ImmutableX, GrindKey) {
    auto res = grindKey("86F3E7293141F20A8BAFF320E8EE4ACCB9D4A4BF2B4D295E8CEE784DB46E0519");
    ASSERT_EQ(res, "5c8c8683596c732541a59e03007b2d30dbbbb873556fe65b5fb63c16688f941");
    auto data = parse_hex(res, true);
    std::cout << hex(data) << std::endl;
}

TEST(ImmutableX, GetPrivateKeySignature) {
    std::string signature = "0x21fbf0696d5e0aa2ef41a2b4ffb623bcaf070461d61cf7251c74161f82fec3a4370854bc0a34b3ab487c1bc021cd318c734c51ae29374f2beb0e6f2dd49b4bf41c";
    auto data = parse_hex(signature);
    auto ethSignature = Ethereum::Signer::signatureDataToStructSimple(data);
    auto seed = store(ethSignature.r);
    auto result = grindKey(hex(seed));
    ASSERT_EQ(result, "766f11e90cd7c7b43085b56da35c781f8c067ac0d578eabdceebc4886435bda");
}

TEST(ImmutableX, GetPrivateKeyFromSignature) {
    std::string address = "0xa76e3eeb2f7143165618ab8feaabcd395b6fac7f";
    std::string signature = "0x5a263fad6f17f23e7c7ea833d058f3656d3fe464baf13f6f5ccba9a2466ba2ce4c4a250231bcac7beb165aec4c9b049b4ba40ad8dd287dc79b92b1ffcf20cdcf1b";
    auto privKey = getPrivateKeyFromRawSignature(signature, address);
    ASSERT_EQ(privKey, "058ab7989d625b1a690400dcbe6e070627adedceff7bd196e58d4791026a8afe");
}

TEST(ImmutableX, GetPublicKeyFromSignature) {
    auto pubKey = getPublicKeyFromPrivateKey("058ab7989d625b1a690400dcbe6e070627adedceff7bd196e58d4791026a8afe");
    ASSERT_EQ(pubKey, "0x2a4c7332c55d6c1c510d24272d1db82878f2302f05b53bcc38695ed5f78fffd");
}


}
