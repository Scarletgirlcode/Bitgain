// Copyright © 2017-2021 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Coin.h"
#include "HexCoding.h"
#include "Base64.h"
#include "proto/Cosmos.pb.h"
#include "Cosmos/Address.h"
#include "Cosmos/Signer.h"
#include "../interface/TWTestUtilities.h"

#include <gtest/gtest.h>
#include <google/protobuf/util/json_util.h>

using namespace TW;
using namespace TW::Cosmos;


TEST(CosmosSigner, SignTxProtobuf) {
    auto input = Proto::SigningInput();
    input.set_signing_mode(Proto::Protobuf);
    input.set_account_number(1037);
    input.set_chain_id("gaia-13003");
    input.set_memo("");
    input.set_sequence(8);

    auto fromAddress = Address("cosmos", parse_hex("BC2DA90C84049370D1B7C528BC164BC588833F21"));
    auto toAddress = Address("cosmos", parse_hex("12E8FE8B81ECC1F4F774EA6EC8DF267138B9F2D9"));

    auto msg = input.add_messages();
    auto& message = *msg->mutable_send_coins_message();
    message.set_from_address(fromAddress.string());
    message.set_to_address(toAddress.string());
    auto amountOfTx = message.add_amounts();
    amountOfTx->set_denom("muon");
    amountOfTx->set_amount("1");

    auto& fee = *input.mutable_fee();
    fee.set_gas(200000);
    auto amountOfFee = fee.add_amounts();
    amountOfFee->set_denom("muon");
    amountOfFee->set_amount("200");

    std::string json;
    google::protobuf::util::MessageToJsonString(input, &json);

    EXPECT_EQ(R"({"signingMode":"Protobuf","accountNumber":"1037","chainId":"gaia-13003","fee":{"amounts":[{"denom":"muon","amount":"200"}],"gas":"200000"},"sequence":"8","messages":[{"sendCoinsMessage":{"fromAddress":"cosmos1hsk6jryyqjfhp5dhc55tc9jtckygx0eph6dd02","toAddress":"cosmos1zt50azupanqlfam5afhv3hexwyutnukeh4c573","amounts":[{"denom":"muon","amount":"1"}]}}]})", json);

    auto privateKey = parse_hex("80e81ea269e66a0a05b11236df7919fb7fbeedba87452d667489d7403a02f005");
    input.set_private_key(privateKey.data(), privateKey.size());

    auto output = Signer::sign(input);

    assertJSONEqual(output.serialized(), "{\"tx_bytes\": \"CowBCokBChwvY29zbW9zLmJhbmsudjFiZXRhMS5Nc2dTZW5kEmkKLWNvc21vczFoc2s2anJ5eXFqZmhwNWRoYzU1dGM5anRja3lneDBlcGg2ZGQwMhItY29zbW9zMXp0NTBhenVwYW5xbGZhbTVhZmh2M2hleHd5dXRudWtlaDRjNTczGgkKBG11b24SATESZQpQCkYKHy9jb3Ntb3MuY3J5cHRvLnNlY3AyNTZrMS5QdWJLZXkSIwohAlcobsPzfTNVe7uqAAsndErJAjqplnyudaGB0f+R+p3FEgQKAggBGAgSEQoLCgRtdW9uEgMyMDAQwJoMGkD54fQAFlekIAnE62hZYl0uQelh/HLv0oQpCciY5Dn8H1SZFuTsrGdu41PH1Uxa4woptCELi/8Ov9yzdeEFAC9H\", \"mode\": \"BROADCAST_MODE_BLOCK\"}");
    EXPECT_EQ(hex(output.signature()), "f9e1f4001657a42009c4eb6859625d2e41e961fc72efd2842909c898e439fc1f549916e4ecac676ee353c7d54c5ae30a29b4210b8bff0ebfdcb375e105002f47");
    EXPECT_EQ(output.json(), "");
    EXPECT_EQ(output.error(), "");
}

TEST(CosmosSigner, SignProtobuf_ErrorMissingMessage) {
    auto input = Proto::SigningInput();
    input.set_signing_mode(Proto::Protobuf);
    input.set_account_number(1037);
    input.set_chain_id("gaia-13003");
    input.set_memo("");
    input.set_sequence(8);

    auto fromAddress = Address("cosmos", parse_hex("BC2DA90C84049370D1B7C528BC164BC588833F21"));
    auto toAddress = Address("cosmos", parse_hex("12E8FE8B81ECC1F4F774EA6EC8DF267138B9F2D9"));

    auto& fee = *input.mutable_fee();
    fee.set_gas(200000);
    auto amountOfFee = fee.add_amounts();
    amountOfFee->set_denom("muon");
    amountOfFee->set_amount("200");

    auto privateKey = parse_hex("80e81ea269e66a0a05b11236df7919fb7fbeedba87452d667489d7403a02f005");
    input.set_private_key(privateKey.data(), privateKey.size());

    auto output = Signer::sign(input);

    EXPECT_EQ(output.error(), "Error: No message found");
    EXPECT_EQ(output.serialized(), "");
    EXPECT_EQ(output.json(), "");
    EXPECT_EQ(hex(output.signature()), "");
}

TEST(CosmosSigner, SignTxJson) {
    auto input = Proto::SigningInput();
    input.set_signing_mode(Proto::JSON); // obsolete
    input.set_account_number(1037);
    input.set_chain_id("gaia-13003");
    input.set_memo("");
    input.set_sequence(8);

    auto fromAddress = Address("cosmos", parse_hex("BC2DA90C84049370D1B7C528BC164BC588833F21"));
    auto toAddress = Address("cosmos", parse_hex("12E8FE8B81ECC1F4F774EA6EC8DF267138B9F2D9"));

    auto msg = input.add_messages();
    auto& message = *msg->mutable_send_coins_message();
    message.set_from_address(fromAddress.string());
    message.set_to_address(toAddress.string());
    auto amountOfTx = message.add_amounts();
    amountOfTx->set_denom("muon");
    amountOfTx->set_amount("1");

    auto& fee = *input.mutable_fee();
    fee.set_gas(200000);
    auto amountOfFee = fee.add_amounts();
    amountOfFee->set_denom("muon");
    amountOfFee->set_amount("200");

    std::string json;
    google::protobuf::util::MessageToJsonString(input, &json);

    EXPECT_EQ(R"({"accountNumber":"1037","chainId":"gaia-13003","fee":{"amounts":[{"denom":"muon","amount":"200"}],"gas":"200000"},"sequence":"8","messages":[{"sendCoinsMessage":{"fromAddress":"cosmos1hsk6jryyqjfhp5dhc55tc9jtckygx0eph6dd02","toAddress":"cosmos1zt50azupanqlfam5afhv3hexwyutnukeh4c573","amounts":[{"denom":"muon","amount":"1"}]}}]})", json);

    auto privateKey = parse_hex("80e81ea269e66a0a05b11236df7919fb7fbeedba87452d667489d7403a02f005");
    input.set_private_key(privateKey.data(), privateKey.size());

    auto output = Signer::sign(input);

    // the sample tx on testnet
    // https://hubble.figment.network/chains/gaia-13003/blocks/142933/transactions/3A9206598C3D2E75A5EC074FD33EA53EB18EC729357F0965971C1C51F812AEA3?format=json
    EXPECT_EQ(R"({"mode":"block","tx":{"fee":{"amount":[{"amount":"200","denom":"muon"}],"gas":"200000"},"memo":"","msg":[{"type":"cosmos-sdk/MsgSend","value":{"amount":[{"amount":"1","denom":"muon"}],"from_address":"cosmos1hsk6jryyqjfhp5dhc55tc9jtckygx0eph6dd02","to_address":"cosmos1zt50azupanqlfam5afhv3hexwyutnukeh4c573"}}],"signatures":[{"pub_key":{"type":"tendermint/PubKeySecp256k1","value":"AlcobsPzfTNVe7uqAAsndErJAjqplnyudaGB0f+R+p3F"},"signature":"/D74mdIGyIB3/sQvIboLTfS9P9EV/fYGrgHZE2/vNj9X6eM6e57G3atljNB+PABnRw3pTk51uXmhCFop8O/ZJg=="}]}})", output.json());

    EXPECT_EQ(hex(output.signature()), "fc3ef899d206c88077fec42f21ba0b4df4bd3fd115fdf606ae01d9136fef363f57e9e33a7b9ec6ddab658cd07e3c0067470de94e4e75b979a1085a29f0efd926");
    EXPECT_EQ(output.serialized(), "");
    EXPECT_EQ(output.error(), "");
}

TEST(CosmosSigner, SignTxJson_WithMode) {
    auto input = Proto::SigningInput();
    input.set_signing_mode(Proto::JSON); // obsolete
    input.set_account_number(1037);
    input.set_chain_id("gaia-13003");
    input.set_memo("");
    input.set_sequence(8);
    input.set_mode(Proto::BroadcastMode::ASYNC);

    auto fromAddress = Address("cosmos", parse_hex("BC2DA90C84049370D1B7C528BC164BC588833F21"));
    auto toAddress = Address("cosmos", parse_hex("12E8FE8B81ECC1F4F774EA6EC8DF267138B9F2D9"));

    auto msg = input.add_messages();
    auto& message = *msg->mutable_send_coins_message();
    message.set_from_address(fromAddress.string());
    message.set_to_address(toAddress.string());
    auto amountOfTx = message.add_amounts();
    amountOfTx->set_denom("muon");
    amountOfTx->set_amount("1");

    auto& fee = *input.mutable_fee();
    fee.set_gas(200000);
    auto amountOfFee = fee.add_amounts();
    amountOfFee->set_denom("muon");
    amountOfFee->set_amount("200");

    auto privateKey = parse_hex("80e81ea269e66a0a05b11236df7919fb7fbeedba87452d667489d7403a02f005");
    input.set_private_key(privateKey.data(), privateKey.size());

    {
        auto output = Signer::sign(input);
        EXPECT_EQ(R"({"mode":"async","tx":{"fee":{"amount":[{"amount":"200","denom":"muon"}],"gas":"200000"},"memo":"","msg":[{"type":"cosmos-sdk/MsgSend","value":{"amount":[{"amount":"1","denom":"muon"}],"from_address":"cosmos1hsk6jryyqjfhp5dhc55tc9jtckygx0eph6dd02","to_address":"cosmos1zt50azupanqlfam5afhv3hexwyutnukeh4c573"}}],"signatures":[{"pub_key":{"type":"tendermint/PubKeySecp256k1","value":"AlcobsPzfTNVe7uqAAsndErJAjqplnyudaGB0f+R+p3F"},"signature":"/D74mdIGyIB3/sQvIboLTfS9P9EV/fYGrgHZE2/vNj9X6eM6e57G3atljNB+PABnRw3pTk51uXmhCFop8O/ZJg=="}]}})", output.json());
        EXPECT_EQ(output.error(), "");
    }
    input.set_mode(Proto::BroadcastMode::SYNC);
    {
        auto output = Signer::sign(input);
        EXPECT_EQ(R"({"mode":"sync","tx":{"fee":{"amount":[{"amount":"200","denom":"muon"}],"gas":"200000"},"memo":"","msg":[{"type":"cosmos-sdk/MsgSend","value":{"amount":[{"amount":"1","denom":"muon"}],"from_address":"cosmos1hsk6jryyqjfhp5dhc55tc9jtckygx0eph6dd02","to_address":"cosmos1zt50azupanqlfam5afhv3hexwyutnukeh4c573"}}],"signatures":[{"pub_key":{"type":"tendermint/PubKeySecp256k1","value":"AlcobsPzfTNVe7uqAAsndErJAjqplnyudaGB0f+R+p3F"},"signature":"/D74mdIGyIB3/sQvIboLTfS9P9EV/fYGrgHZE2/vNj9X6eM6e57G3atljNB+PABnRw3pTk51uXmhCFop8O/ZJg=="}]}})", output.json());
        EXPECT_EQ(output.error(), "");
    }
}

TEST(CosmosSigner, SignIbcTransferProtobuf_817101) {
    auto input = Proto::SigningInput();
    input.set_signing_mode(Proto::Protobuf);
    input.set_account_number(546179);
    input.set_chain_id("cosmoshub-4");
    input.set_sequence(2);

    Address fromAddress;
    EXPECT_TRUE(Address::decode("cosmos1mky69cn8ektwy0845vec9upsdphktxt03gkwlx", fromAddress));
    Address toAddress;
    EXPECT_TRUE(Address::decode("osmo18s0hdnsllgcclweu9aymw4ngktr2k0rkvn7jmn", toAddress));

    auto msg = input.add_messages();
    auto& message = *msg->mutable_transfer_tokens_message();
    message.set_source_port("transfer");
    message.set_source_channel("channel-141");
    message.set_sender(fromAddress.string());
    message.set_receiver(toAddress.string());
    message.mutable_token()->set_denom("uatom");
    message.mutable_token()->set_amount("100000"); // 0.1 ATOM
    message.mutable_timeout_height()->set_revision_number(1);
    message.mutable_timeout_height()->set_revision_height(8800000);

    auto& fee = *input.mutable_fee();
    fee.set_gas(500000);
    auto amountOfFee = fee.add_amounts();
    amountOfFee->set_denom("uatom");
    amountOfFee->set_amount("12500");

    auto privateKey = parse_hex("8bbec3772ddb4df68f3186440380c301af116d1422001c1877d6f5e4dba8c8af");
    EXPECT_EQ(Cosmos::Address(TWCoinTypeCosmos, PrivateKey(privateKey).getPublicKey(TWPublicKeyTypeSECP256k1)).string(), "cosmos1mky69cn8ektwy0845vec9upsdphktxt03gkwlx");
    input.set_private_key(privateKey.data(), privateKey.size());

    auto output = Signer::sign(input);

    // real-world tx: https://www.mintscan.io/cosmos/txs/817101F3D96314AD028733248B28BAFAD535024D7D2C8875D3FE31DC159F096B
    // curl -H 'Content-Type: application/json' --data-binary '{"tx_bytes": "Cr4BCr...1yKOU=", "mode": "BROADCAST_MODE_BLOCK"}' https://api.cosmos.network/cosmos/tx/v1beta1/txs 
    // also similar TX: BCDAC36B605576C8182C2829C808B30A69CAD4959D5ED1E6FF9984ABF280D603
    assertJSONEqual(output.serialized(), "{\"tx_bytes\": \"Cr4BCrsBCikvaWJjLmFwcGxpY2F0aW9ucy50cmFuc2Zlci52MS5Nc2dUcmFuc2ZlchKNAQoIdHJhbnNmZXISC2NoYW5uZWwtMTQxGg8KBXVhdG9tEgYxMDAwMDAiLWNvc21vczFta3k2OWNuOGVrdHd5MDg0NXZlYzl1cHNkcGhrdHh0MDNna3dseCorb3NtbzE4czBoZG5zbGxnY2Nsd2V1OWF5bXc0bmdrdHIyazBya3ZuN2ptbjIHCAEQgI6ZBBJoClAKRgofL2Nvc21vcy5jcnlwdG8uc2VjcDI1NmsxLlB1YktleRIjCiEC7O9c5DejAsZ/lUaN5LMfNukR9GfX5qUrQcHhPh1WNkkSBAoCCAEYAhIUCg4KBXVhdG9tEgUxMjUwMBCgwh4aQK0HIWdFMk+C6Gi1KG/vELe1ffcc1aEWUIqz2t/ZhwqNNHxUUSp27wteiugHEMVTEIOBhs84t2gIcT/nD/1yKOU=\", \"mode\": \"BROADCAST_MODE_BLOCK\"}");
    EXPECT_EQ(hex(output.signature()), "ad07216745324f82e868b5286fef10b7b57df71cd5a116508ab3dadfd9870a8d347c54512a76ef0b5e8ae80710c55310838186cf38b76808713fe70ffd7228e5");
    EXPECT_EQ(output.json(), "");
    EXPECT_EQ(output.error(), "");
}
