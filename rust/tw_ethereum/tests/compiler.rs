// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use std::borrow::Cow;
use tw_coin_entry::coin_entry_ext::CoinEntryExt;
use tw_coin_entry::error::SigningErrorType;
use tw_coin_entry::modules::input_builder::BuildSigningInputArgs;
use tw_coin_entry::test_utils::empty_context::EmptyCoinContext;
use tw_encoding::hex;
use tw_ethereum::entry::EthereumEntry;
use tw_keypair::ecdsa::secp256k1;
use tw_keypair::tw;
use tw_number::U256;
use tw_proto::Ethereum::Proto;
use tw_proto::TxCompiler::Proto as CompilerProto;
use tw_proto::{deserialize, serialize};

#[test]
fn test_external_signature_sign() {
    let args = BuildSigningInputArgs {
        from: "0x9d8A62f656a8d1615C1294fd71e9CFb3E4855A4F".to_string(),
        to: "0x3535353535353535353535353535353535353535".to_string(),
        amount: "1000000000000000000".to_string(),
        asset: "ETH".to_string(),
        memo: "".to_string(),
        chain_id: "".to_string(),
    };
    let res = EthereumEntry
        .build_signing_input(&EmptyCoinContext, args.clone())
        .expect("!build_signing_input")
        .expect("'build_signing_input' should return something");
    let mut input: Proto::SigningInput =
        deserialize(res.as_slice()).expect("Coin entry returned an invalid output");

    let transfer = Proto::mod_Transaction::Transfer {
        amount: U256::encode_be_compact(1_000_000_000_000_000_000),
        data: Cow::default(),
    };
    let expected_input = Proto::SigningInput {
        chain_id: U256::encode_be_compact(1),
        to_address: Cow::from(args.to),
        transaction: Some(Proto::Transaction {
            transaction_oneof: Proto::mod_Transaction::OneOftransaction_oneof::transfer(transfer),
        }),
        ..Proto::SigningInput::default()
    };
    assert_eq!(input, expected_input);

    // Set a few other values.
    input.nonce = U256::encode_be_compact(11);
    input.gas_price = U256::encode_be_compact(20_000_000_000);
    input.gas_limit = U256::encode_be_compact(21_000);
    input.tx_mode = Proto::TransactionMode::Legacy;

    // Step 2: Obtain preimage hash
    let input_data = serialize(&input).unwrap();
    let preimage_data = EthereumEntry
        .preimage_hashes(&EmptyCoinContext, &input_data)
        .expect("!preimage_hashes");
    let preimage: CompilerProto::PreSigningOutput =
        deserialize(&preimage_data).expect("Coin entry returned an invalid output");

    assert_eq!(preimage.error, SigningErrorType::OK);
    assert!(preimage.error_message.is_empty());
    assert_eq!(
        hex::encode(&preimage.data_hash, false),
        "15e180a6274b2f6a572b9b51823fce25ef39576d10188ecdcd7de44526c47217"
    );

    // Simulate signature, normally obtained from signature server
    let public_key = secp256k1::PublicKey::try_from("044bc2a31265153f07e70e0bab08724e6b85e217f8cd628ceb62974247bb493382ce28cab79ad7119ee1ad3ebcdb98a16805211530ecc6cfefa1b88e6dff99232a").unwrap();
    let public_key = tw::PublicKey::Secp256k1Extended(public_key);
    let signature = hex::decode("360a84fb41ad07f07c845fedc34cde728421803ebbaae392fc39c116b29fc07b53bd9d1376e15a191d844db458893b928f3efbfee90c9febf51ab84c9796677900").unwrap();

    // Verify signature (pubkey & hash & signature)
    assert!(public_key.verify(&signature, &preimage.data_hash));

    // Step 3: Compile transaction info
    let input_data = serialize(&input).unwrap();
    let output_data = EthereumEntry
        .compile(
            &EmptyCoinContext,
            &input_data,
            vec![signature],
            vec![public_key.to_bytes()],
        )
        .expect("!compile");
    let output: Proto::SigningOutput =
        deserialize(&output_data).expect("Coin entry returned an invalid output");

    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());
    let expected_encoded = "f86c0b8504a817c800825208943535353535353535353535353535353535353535880de0b6b3a76400008025a0360a84fb41ad07f07c845fedc34cde728421803ebbaae392fc39c116b29fc07ba053bd9d1376e15a191d844db458893b928f3efbfee90c9febf51ab84c97966779";
    assert_eq!(hex::encode(output.encoded, false), expected_encoded);

    // Double check: check if simple signature process gives the same result. Note that private
    // keys were not used anywhere up to this point.
    input.private_key = Cow::from(
        hex::decode("4646464646464646464646464646464646464646464646464646464646464646").unwrap(),
    );

    let input_data = serialize(&input).unwrap();
    let output_data = EthereumEntry
        .sign(&EmptyCoinContext, &input_data)
        .expect("!output_data");
    let output: Proto::SigningOutput =
        deserialize(&output_data).expect("Coin entry returned an invalid output");
    assert_eq!(hex::encode(output.encoded, false), expected_encoded);
}
