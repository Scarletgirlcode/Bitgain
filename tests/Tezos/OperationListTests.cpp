// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Tezos/Address.h"
#include "Tezos/BinaryCoding.h"
#include "Tezos/OperationList.h"
#include "proto/Tezos.pb.h"
#include "HexCoding.h"

#include <gtest/gtest.h>

using namespace TW::Tezos;
using namespace TW::Tezos::Proto;

TEST(TezosOperationList, ForgeBranch) {
    auto input = TW::Tezos::OperationList("BMNY6Jkas7BzKb7wDLCFoQ4YxfYoieU7Xmo1ED3Y9Lo3ZvVGdgW");
    auto expected = "da8eb4f57f98a647588b47d29483d1edfdbec1428c11609cee0da6e0f27cfc38";

    ASSERT_EQ(input.forgeBranch(), parse_hex(expected));
}

TEST(TezosOperationList, ForgeOperationList_TransactionOnly) {
    auto branch = "BL8euoCWqNCny9AR3AKjnpi38haYMxjei1ZqNHuXMn19JSQnoWp";
    auto op_list = TW::Tezos::OperationList(branch);

    auto transactionOperationData = new TW::Tezos::Proto::TransactionOperationData();
    transactionOperationData -> set_amount(1);
    transactionOperationData -> set_destination("tz1Yju7jmmsaUiG9qQLoYv35v5pHgnWoLWbt");

    auto transactionOperation = TW::Tezos::Proto::Operation();
    transactionOperation.set_source("tz1XVJ8bZUXs7r5NV8dHvuiBhzECvLRLR3jW");
    transactionOperation.set_fee(1272);
    transactionOperation.set_counter(30738);
    transactionOperation.set_gas_limit(10100);
    transactionOperation.set_storage_limit(257);
    transactionOperation.set_kind(TW::Tezos::Proto::Operation::TRANSACTION);
    transactionOperation.set_allocated_transaction_operation_data(transactionOperationData);

    op_list.addOperation(transactionOperation);

    auto expected = "3756ef37b1be849e3114643f0aa5847cabf9a896d3bfe4dd51448de68e91da01080081faa75f741ef614b0e35fcc8c90dfa3b0b95721f80992f001f44e810201008fb5cea62d147c696afd9a93dbce962f4c8a9c9100";
    auto forged = op_list.forge();
    ASSERT_EQ(hex(forged.begin(), forged.end()), expected);
}

TEST(TezosOperationList, ForgeOperationList_RevealOnly) {
      auto branch = "BL8euoCWqNCny9AR3AKjnpi38haYMxjei1ZqNHuXMn19JSQnoWp";
      auto op_list = TW::Tezos::OperationList(branch);

      PublicKey publicKey = parsePublicKey("edpku9ZF6UUAEo1AL3NWy1oxHLL6AfQcGYwA5hFKrEKVHMT3Xx889A");

      auto revealOperationData = new TW::Tezos::Proto::RevealOperationData();
      revealOperationData -> set_public_key(publicKey.bytes.data(), publicKey.bytes.size());

      auto revealOperation = TW::Tezos::Proto::Operation();
      revealOperation.set_source("tz1XVJ8bZUXs7r5NV8dHvuiBhzECvLRLR3jW");
      revealOperation.set_fee(1272);
      revealOperation.set_counter(30738);
      revealOperation.set_gas_limit(10100);
      revealOperation.set_storage_limit(257);
      revealOperation.set_kind(TW::Tezos::Proto::Operation::REVEAL);
      revealOperation.set_allocated_reveal_operation_data(revealOperationData);

      op_list.addOperation(revealOperation);
      auto expected = "3756ef37b1be849e3114643f0aa5847cabf9a896d3bfe4dd51448de68e91da01070081faa75f741ef614b0e35fcc8c90dfa3b0b95721f80992f001f44e8102429a986c8072a40a1f3a3e2ab5a5819bb1b2fb69993c5004837815b9dc55923e";
      auto forged = op_list.forge();

      ASSERT_EQ(hex(forged.begin(), forged.end()), expected);
}

TEST(TezosOperationList, ForgeOperationList_Delegation_ClearDelegate) {
    auto branch = "BLGJfQDFEYZBRLj5GSHskj8NPaRYhk7Kx5WAfdcDucD3q98WdeW";
    auto op_list = TW::Tezos::OperationList(branch);

    auto delegationOperationData = new TW::Tezos::Proto::DelegationOperationData();
    delegationOperationData -> set_delegate("");

    auto delegationOperation = TW::Tezos::Proto::Operation();
    delegationOperation.set_source("KT1D5jmrBD7bDa3jCpgzo32FMYmRDdK2ihka");
    delegationOperation.set_fee(1257);
    delegationOperation.set_counter(67);
    delegationOperation.set_gas_limit(10000);
    delegationOperation.set_storage_limit(0);
    delegationOperation.set_kind(TW::Tezos::Proto::Operation::DELEGATION);
    delegationOperation.set_allocated_delegation_operation_data(delegationOperationData);

    op_list.addOperation(delegationOperation);

    auto expected = "48b63d801fa824013a195f7885ba522503c59e0580f7663e15c52f03ccc935e60a00315b1206ec00b1b1e64cc3b8b93059f58fa2fc39e90943904e0000";
    ASSERT_EQ(hex(op_list.forge()), hex(parse_hex(expected)));
}

TEST(TezosOperationList, ForgeOperationList_Delegation_AddDelegate) {
    auto branch = "BLa4GrVQTxUgQWbHv6cF7RXWSGzHGPbgecpQ795R3cLzw4cGfpD";
    auto op_list = TW::Tezos::OperationList(branch);

    auto delegationOperationData = new TW::Tezos::Proto::DelegationOperationData();
    delegationOperationData -> set_delegate("tz1dYUCcrorfCoaQCtZaxi1ynGrP3prTZcxS");

    auto delegationOperation = TW::Tezos::Proto::Operation();
    delegationOperation.set_source("KT1D5jmrBD7bDa3jCpgzo32FMYmRDdK2ihka");
    delegationOperation.set_fee(1257);
    delegationOperation.set_counter(68);
    delegationOperation.set_gas_limit(10000);
    delegationOperation.set_storage_limit(0);
    delegationOperation.set_kind(TW::Tezos::Proto::Operation::DELEGATION);
    delegationOperation.set_allocated_delegation_operation_data(delegationOperationData);
    
    op_list.addOperation(delegationOperation);
    auto expected = "7105102c032807994dd9b5edf219261896a559876ca16cbf9d31dbe3612b89f20a00315b1206ec00b1b1e64cc3b8b93059f58fa2fc39e90944904e00ff00c4650fd609f88c67356e5fe01e37cd3ff654b18c";
    auto forged = op_list.forge();
    ASSERT_EQ(hex(forged.begin(), forged.end()), expected);
}

TEST(TezosOperationList, ForgeOperationList_TransactionAndReveal) {
    auto branch = "BL8euoCWqNCny9AR3AKjnpi38haYMxjei1ZqNHuXMn19JSQnoWp";
    auto op_list = TW::Tezos::OperationList(branch);
    
    PublicKey publicKey = parsePublicKey("edpku9ZF6UUAEo1AL3NWy1oxHLL6AfQcGYwA5hFKrEKVHMT3Xx889A");

    auto revealOperationData = new TW::Tezos::Proto::RevealOperationData();
    revealOperationData -> set_public_key(publicKey.bytes.data(), publicKey.bytes.size());

    auto revealOperation = TW::Tezos::Proto::Operation();
    revealOperation.set_source("tz1XVJ8bZUXs7r5NV8dHvuiBhzECvLRLR3jW");
    revealOperation.set_fee(1272);
    revealOperation.set_counter(30738);
    revealOperation.set_gas_limit(10100);
    revealOperation.set_storage_limit(257);
    revealOperation.set_kind(TW::Tezos::Proto::Operation::REVEAL);
    revealOperation.set_allocated_reveal_operation_data(revealOperationData);
    
    auto transactionOperationData = new TW::Tezos::Proto::TransactionOperationData();
    transactionOperationData -> set_amount(1);
    transactionOperationData -> set_destination("tz1XVJ8bZUXs7r5NV8dHvuiBhzECvLRLR3jW");

    auto transactionOperation = TW::Tezos::Proto::Operation();
    transactionOperation.set_source("tz1XVJ8bZUXs7r5NV8dHvuiBhzECvLRLR3jW");
    transactionOperation.set_fee(1272);
    transactionOperation.set_counter(30739);
    transactionOperation.set_gas_limit(10100);
    transactionOperation.set_storage_limit(257);
    transactionOperation.set_kind(TW::Tezos::Proto::Operation::TRANSACTION);
    transactionOperation.set_allocated_transaction_operation_data(transactionOperationData);

    op_list.addOperation(revealOperation);
    op_list.addOperation(transactionOperation);

    auto expected = "3756ef37b1be849e3114643f0aa5847cabf9a896d3bfe4dd51448de68e91da01070081faa75f741ef614b0e35fcc8c90dfa3b0b95721f80992f001f44e8102429a986c8072a40a1f3a3e2ab5a5819bb1b2fb69993c5004837815b9dc55923e080081faa75f741ef614b0e35fcc8c90dfa3b0b95721f80993f001f44e8102010081faa75f741ef614b0e35fcc8c90dfa3b0b9572100";
    auto forged = op_list.forge();

    ASSERT_EQ(hex(forged.begin(), forged.end()), expected);
}
