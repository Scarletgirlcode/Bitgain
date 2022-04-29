// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include <TrustWalletCore/TWAnySigner.h>
#include <TrustWalletCore/TWBitcoinSigHashType.h>
#include <TrustWalletCore/TWCoinType.h>

#include "Bitcoin/Signer.h"
#include "Bitcoin/Address.h"
#include "Bitcoin/TransactionPlan.h"
#include "Bitcoin/TransactionSigner.h"
#include "HexCoding.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "HexCoding.h"

#include "../interface/TWTestUtilities.h"
#include <gtest/gtest.h>

using namespace TW;
using namespace TW::Bitcoin;

TEST(TWAnySignerVerge, Sign) {
    const int64_t amount = 1500000000;
    const int64_t fee = 2000000;
    const std::string toAddress = "DQYMMpqPrnWYZaikKGTQqk5ydUaQw8nkdD";

    auto input = Bitcoin::Proto::SigningInput();
    input.set_hash_type(TWBitcoinSigHashTypeAll);
    input.set_amount(amount);
    input.set_byte_fee(1);
    input.set_to_address(toAddress);
    input.set_change_address("DAkEo5pNELZav7MRwBfEwHRG1aChgSUw6c");
    input.set_coin_type(TWCoinTypeVerge);

    auto txHash0 = parse_hex("a5a6e147da0f1b3f6dfd1081f91b0c6e31f030ae66c4be4cf4b0db0ac8b2407d");
    std::reverse(txHash0.begin(), txHash0.end());

    auto utxo0 = input.add_utxo();
    utxo0->mutable_out_point()->set_hash(txHash0.data(), txHash0.size());
    utxo0->mutable_out_point()->set_index(0);
    utxo0->mutable_out_point()->set_sequence(4294967294);
    utxo0->set_amount(2500000000);

    auto utxoKey0 = PrivateKey(parse_hex("693dfe6f3ed717573eb10c24ebe5eb592fa3c239245cd499c487eb7b8ea7ed3a"));
    auto script0 = Bitcoin::Script::lockScriptForAddress("DRyNFvJaybnF22UfMS6NR1Qav3mqxPj86E", TWCoinTypeVerge);
    ASSERT_EQ(hex(script0.bytes), "76a914e4839a523f120882d11eb3dda13a18e11fdcbd4a88ac");
    utxo0->set_script(script0.bytes.data(), script0.bytes.size());
    input.add_private_key(utxoKey0.bytes.data(), utxoKey0.bytes.size());

    Bitcoin::Proto::TransactionPlan plan;
    {
        ANY_PLAN(input, plan, TWCoinTypeVerge);

        plan.set_amount(amount);
        plan.set_fee(fee);
        plan.set_change(980000000);

        *input.mutable_plan() = plan;
    }

    Bitcoin::Proto::SigningOutput output;
    {
        ANY_SIGN(input, TWCoinTypeVerge);
        ASSERT_EQ(output.error(), Common::Proto::OK);
    }

    // Sign
    ASSERT_EQ(hex(output.encoded()),
        "01000000017d40b2c80adbb0f44cbec466ae30f0316e0c1bf98110fd6d3f1b0fda47e1a6a5000000006a47304402201b95a86afa0b4355bbbf7d38b3d8c31aee36e95730efedf903673c8fd6c778a502207c9e885a50e356c4a6101f41a3f36fb2a4a75feafe50684c456e51e6d3f544aa01210220ee0423797a856fdd2e85876a60bf10f8696e6ae83e744f498f2173237fe23dfeffffff02002f6859000000001976a914d4d05406c3ca73cf887911f80c852a1c0773615088ac009d693a000000001976a9143d7e143a8b3c8a4aa2f51104da380edeb6c3fc2088ac00000000"
    );
}
