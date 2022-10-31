// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Hedera/Signer.h"
#include "Hedera/Address.h"
#include "HexCoding.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "Hedera/Protobuf/basic_types.pb.h"
#include "Hedera/Protobuf/crypto_transfer.pb.h"
#include "Hedera/Protobuf/transaction_body.pb.h"

#include <gtest/gtest.h>

namespace TW::Hedera::tests {

TEST(HederaSigner, Sign) {
    // Successfully broadcasted: https://hashscan.io/testnet/transaction/0.0.48694347-1667222879-749068449?t=1667222891.440398729&p=1
    Proto::SigningInput input;
    auto privateKey = PrivateKey(parse_hex("e87a5584c0173263e138db689fdb2a7389025aaae7cb1a18a1017d76012130e8"));
    input.set_private_key(privateKey.bytes.data(), privateKey.bytes.size());
    auto* body = input.mutable_body();

    // memo doesn't work yet
    *body->mutable_memo() = "";
    *body->mutable_nodeaccountid() = "0.0.9";
    body->set_transactionfee(100000000);
    body->set_transactionvalidduration(120);
    auto* transferMsg = body->mutable_transfer();
    transferMsg->set_from("0.0.48694347");
    transferMsg->set_to("0.0.48462050");
    transferMsg->set_amount(100000000);

    auto* transactionID = body->mutable_transactionid();
    transactionID->mutable_transactionvalidstart()->set_seconds(1667222879);
    transactionID->mutable_transactionvalidstart()->set_nanos(749068449);
    transactionID->set_accountid("0.0.48694347");

    auto result = Signer::sign(input);
    ASSERT_EQ(hex(result.encoded()), "0a440a150a0c08df9aff9a0610a1c197e502120518cb889c17120218091880c2d72f22020878721e0a1c0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c1710ff83af5f12660a640a205d3a70d08b2beafb72c7a68986b3ff819a306078b8c359d739e4966e82e6d40e1a40612589c3b15f1e3ed6084b5a3a5b1b81751578cac8d6c922f31731b3982a5bac80a22558b2197276f5bae49b62503a4d39448ceddbc5ef3ba9bee4c0f302f70c");
}

TEST(HederaSigner, SignWithMemo) {
    // Successfully broadcasted: https://hashscan.io/testnet/transaction/0.0.48694347-1667227300-854561449?t=1667227312.554926003
    Proto::SigningInput input;
    auto privateKey = PrivateKey(parse_hex("e87a5584c0173263e138db689fdb2a7389025aaae7cb1a18a1017d76012130e8"));
    input.set_private_key(privateKey.bytes.data(), privateKey.bytes.size());
    auto* body = input.mutable_body();

    // memo doesn't work yet
    *body->mutable_memo() = "wallet core";
    *body->mutable_nodeaccountid() = "0.0.7";
    body->set_transactionfee(100000000);
    body->set_transactionvalidduration(120);
    auto* transferMsg = body->mutable_transfer();
    transferMsg->set_from("0.0.48694347");
    transferMsg->set_to("0.0.48462050");
    transferMsg->set_amount(100000000);

    auto* transactionID = body->mutable_transactionid();
    transactionID->mutable_transactionvalidstart()->set_seconds(1667227300);
    transactionID->mutable_transactionvalidstart()->set_nanos(854561449);
    transactionID->set_accountid("0.0.48694347");

    auto result = Signer::sign(input);
    ASSERT_EQ(hex(result.encoded()), "0a510a150a0c08a4bdff9a0610a9a5be9703120518cb889c17120218071880c2d72f22020878320b77616c6c657420636f7265721e0a1c0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c1710ff83af5f12660a640a205d3a70d08b2beafb72c7a68986b3ff819a306078b8c359d739e4966e82e6d40e1a40ee1764c9acf79b68a675c1a78c8c43cb7d136f5f230b48b44992ad3e7ba87a8256758b823120a76142e58b94f082a0551000cf68cd3336fc4393c6b2191d8603");
}

TEST(HederaSigner, ProtoTestsTransferList) {
    auto transferList = proto::TransferList();
    auto* to = transferList.add_accountamounts();
    to->set_amount(100000000);
    auto accountIdTo = to->mutable_accountid();
    accountIdTo->set_shardnum(0);
    accountIdTo->set_realmnum(0);
    accountIdTo->set_accountnum(48462050);

    auto encoded = hex(transferList.SerializeAsString());
    ASSERT_EQ(encoded, "0a0c0a0518e2f18d17108084af5f");
}

TEST(HederaSigner, ProtoTestsDoubleTransferList) {
    auto transferList = proto::TransferList();

    {
        auto* to = transferList.add_accountamounts();
        to->set_amount(100000000);
        auto* accountIdTo = to->mutable_accountid();
        accountIdTo->set_shardnum(0);
        accountIdTo->set_realmnum(0);
        accountIdTo->set_accountnum(48462050);
    }

    {
        auto* from = transferList.add_accountamounts();
        from->set_amount(-100000000);
        auto* accountIdFrom = from->mutable_accountid();
        accountIdFrom->set_shardnum(0);
        accountIdFrom->set_realmnum(0);
        accountIdFrom->set_accountnum(48694347);
    }

    auto encoded = hex(transferList.SerializeAsString());
    ASSERT_EQ(encoded, "0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c17108084af5f");
}

TEST(HederaSigner, ProtoTestsCryptoTransfer) {
    auto transferList = proto::TransferList();

    {
        auto* to = transferList.add_accountamounts();
        to->set_amount(100000000);
        auto* accountIdTo = to->mutable_accountid();
        accountIdTo->set_shardnum(0);
        accountIdTo->set_realmnum(0);
        accountIdTo->set_accountnum(48462050);
    }

    {
        auto* from = transferList.add_accountamounts();
        from->set_amount(-100000000);
        auto* accountIdFrom = from->mutable_accountid();
        accountIdFrom->set_shardnum(0);
        accountIdFrom->set_realmnum(0);
        accountIdFrom->set_accountnum(48694347);
    }

    auto cryptoTransfer = proto::CryptoTransferTransactionBody();
    *cryptoTransfer.mutable_transfers() = transferList;

    auto encoded = hex(cryptoTransfer.SerializeAsString());
    ASSERT_EQ(encoded, "0a1c0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c1710ff83af5f");
}

TEST(HederaSigner, ProtoTestsTransactionBody) {
    auto transferList = proto::TransferList();

    {
        auto* to = transferList.add_accountamounts();
        to->set_amount(100000000);
        auto* accountIdTo = to->mutable_accountid();
        accountIdTo->set_shardnum(0);
        accountIdTo->set_realmnum(0);
        accountIdTo->set_accountnum(48462050);
    }

    {
        auto* from = transferList.add_accountamounts();
        from->set_amount(-100000000);
        auto* accountIdFrom = from->mutable_accountid();
        accountIdFrom->set_shardnum(0);
        accountIdFrom->set_realmnum(0);
        accountIdFrom->set_accountnum(48694347);
    }

    auto cryptoTransfer = proto::CryptoTransferTransactionBody();
    *cryptoTransfer.mutable_transfers() = transferList;

    auto transactionBody = proto::TransactionBody();
    //transactionBody.set_memo("wallet core");
    *transactionBody.mutable_cryptotransfer() = cryptoTransfer;
    transactionBody.set_transactionfee(100000000);
    transactionBody.mutable_nodeaccountid()->set_accountnum(9);
    transactionBody.mutable_transactionvalidduration()->set_seconds(120);
    auto& transactionID = *transactionBody.mutable_transactionid();
    transactionID.mutable_accountid()->set_accountnum(48694347);
    transactionID.mutable_transactionvalidstart()->set_nanos(749068449);
    transactionID.mutable_transactionvalidstart()->set_seconds(1667222879);

    auto encoded = hex(transactionBody.SerializeAsString());

    ASSERT_EQ(encoded, "0a150a0c08df9aff9a0610a1c197e502120518cb889c17120218091880c2d72f22020878721e0a1c0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c1710ff83af5f");
}

TEST(HederaSigner, ProtoTestsTransactionBodyWithMemo) {
    auto transferList = proto::TransferList();
    {
        auto* to = transferList.add_accountamounts();
        to->set_amount(100000000);
        auto* accountIdTo = to->mutable_accountid();
        accountIdTo->set_shardnum(0);
        accountIdTo->set_realmnum(0);
        accountIdTo->set_accountnum(48462050);
    }

    {
        auto* from = transferList.add_accountamounts();
        from->set_amount(-100000000);
        auto* accountIdFrom = from->mutable_accountid();
        accountIdFrom->set_shardnum(0);
        accountIdFrom->set_realmnum(0);
        accountIdFrom->set_accountnum(48694347);
    }

    auto cryptoTransfer = proto::CryptoTransferTransactionBody();
    *cryptoTransfer.mutable_transfers() = transferList;

    auto transactionBody = proto::TransactionBody();
    transactionBody.set_memo("wallet core");
    *transactionBody.mutable_cryptotransfer() = cryptoTransfer;
    transactionBody.set_transactionfee(100000000);
    transactionBody.mutable_nodeaccountid()->set_accountnum(3);
    transactionBody.mutable_transactionvalidduration()->set_seconds(120);
    auto& transactionID = *transactionBody.mutable_transactionid();
    transactionID.mutable_accountid()->set_accountnum(48694347);
    transactionID.mutable_transactionvalidstart()->set_nanos(942876449);
    transactionID.mutable_transactionvalidstart()->set_seconds(1667215034);

    auto encoded = hex(transactionBody.SerializeAsString());
    ASSERT_EQ(encoded, "0a150a0c08baddfe9a0610a1ceccc103120518cb889c17120218031880c2d72f22020878320b77616c6c657420636f7265721e0a1c0a0c0a0518e2f18d17108084af5f0a0c0a0518cb889c1710ff83af5f");
}





} // namespace TW::Hedera::tests
