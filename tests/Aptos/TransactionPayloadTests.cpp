// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include <Aptos/TransactionPayload.h>
#include <HexCoding.h>
#include <gtest/gtest.h>

namespace TW::Aptos::tests {

TEST(AptosTransactionPayload, PayLoadBasis) {
    // strict equivalent of the following rust test
    // #[test]
    // fn test_payload() {
    //    let from = AccountAddress::from_str("0xeeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b").unwrap();
    //    let to = AccountAddress::from_str("0xeeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b").unwrap();
    //    let st = StructTag {
    //        address: AccountAddress::ONE,
    //        module: Identifier::from_utf8(("aptos_coin".as_bytes()).to_vec()).unwrap(),
    //        name: Identifier::from_utf8(("AptosCoin".as_bytes()).to_vec()).unwrap(),
    //        type_params: vec![],
    //    };
    //    let tag = TypeTag::Struct(st);
    //    let val = TransactionPayload::EntryFunction(EntryFunction::new(
    //        ModuleId::new(AccountAddress::ONE, Identifier::new("coin").unwrap()),
    //        Identifier::new("transfer").unwrap(),
    //        vec![tag],
    //        vec![
    //            bcs::to_bytes(&from).unwrap(),
    //            bcs::to_bytes(&to).unwrap(),
    //        ],
    //    ));
    //    let val = hex::encode(bcs::to_bytes(&val).unwrap());
    //    assert_eq!(val, "02000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220eeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b20eeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b");
    //}
    ModuleId module(gAddressOne, "coin");
    Address from("0xeeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b");
    Address to("0xeeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b");
    BCS::Serializer serializer;
    serializer << from;
    std::vector<Data> args;
    args.emplace_back(serializer.bytes);
    serializer.clear();
    serializer << to;
    args.emplace_back(serializer.bytes);
    TransactionPayload payload = EntryFunction(module, "transfer", {gTransferTag}, args);
    ASSERT_EQ(std::get<EntryFunction>(payload).module().name(), "coin");
    ASSERT_EQ(std::get<EntryFunction>(payload).module().shortString(), "0x1::coin");
    serializer.clear();
    serializer << payload;
    ASSERT_EQ(hex(serializer.bytes), "02000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220eeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b20eeff357ea5c1a4e7bc11b2b17ff2dc2dcca69750bfef1e1ebcaccf8c8018175b");
}

} // namespace TW::Aptos::tests
