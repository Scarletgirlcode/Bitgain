// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use move_core_types::language_storage::TypeTag;
use tw_aptos::liquid_staking::LiquidStakingOperation;
use tw_aptos::nft::NftOperation;
use tw_aptos::signer::{Signer, StandardAptosContext};
use tw_coin_entry::error::SigningErrorType;
use tw_encoding::hex;
use tw_proto::Aptos::Proto;
use tw_proto::Aptos::Proto::{SigningInput, SigningOutput};


pub struct AccountCreation {
    to: String,
}

pub struct Transfer {
    to: String,
    amount: u64,
}

pub struct TokenTransfer {
    transfer: Transfer,
    tag: TypeTag,
}

pub struct RegisterToken {
    coin_type: TypeTag,
}

pub enum OpsDetails {
    RegisterToken(RegisterToken),
    LiquidStakingOps(LiquidStakingOperation),
    AccountCreation(AccountCreation),
    Transfer(Transfer),
    TokenTransfer(TokenTransfer),
    ImplicitTokenTransfer(TokenTransfer),
    NftOps(NftOperation),
}

fn setup_proto_transaction<'a>(sender: &'a str,
                               keypair_str: &'a str,
                               transaction_type: &'a str,
                               sequence_number: i64,
                               chain_id: u32,
                               max_gas_amount: u64,
                               timestamp: u64,
                               gas_unit_price: u64,
                               any_encoded: &'a str,
                               ops_details: Option<OpsDetails>) -> SigningInput<'a> {
    let private = hex::decode(keypair_str).unwrap();

    let payload: Proto::mod_SigningInput::OneOftransaction_payload = match transaction_type {
        "transfer" => {
            if let OpsDetails::Transfer(transfer) = ops_details.unwrap() {
                Proto::mod_SigningInput::OneOftransaction_payload::transfer(Proto::TransferMessage {
                    to: transfer.to.into(),
                    amount: transfer.amount,
                })
            } else {
                panic!("Unsupported arguments")
            }
        }
        "create_account" => {
            if let OpsDetails::AccountCreation(account) = ops_details.unwrap() {
                Proto::mod_SigningInput::OneOftransaction_payload::create_account(Proto::CreateAccountMessage {
                    auth_key: account.to.into(),
                })
            } else {
                panic!("Unsupported arguments")
            }
        }
        _ => { todo!() }
    };


    let input = SigningInput {
        chain_id,
        sender: sender.into(),
        sequence_number,
        max_gas_amount,
        gas_unit_price,
        expiration_timestamp_secs: timestamp,
        private_key: private.into(),
        any_encoded: any_encoded.into(),
        transaction_payload: payload,
    };

    input
}

fn test_tx_result(
    output: SigningOutput,
    expected_raw_txn_bytes_str: &str,
    expected_signature_str: &str,
    expected_encoded_txn_str: &str,
    json_literal: &str) {
    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());


    assert_eq!(hex::encode(output.raw_txn.to_vec(), false), expected_raw_txn_bytes_str);
    assert_eq!(hex::encode(output.authenticator.unwrap().signature.to_vec(), false), expected_signature_str);
    assert_eq!(hex::encode(output.encoded.to_vec(), false), expected_encoded_txn_str);

    let json_value_expected: serde_json::Value = serde_json::from_str(json_literal).unwrap();
    let json_value: serde_json::Value = serde_json::from_str(output.json.as_ref()).unwrap();
    assert_eq!(json_value, json_value_expected);
}

#[test]
fn test_aptos_sign_transaction_transfer() {
    let input = setup_proto_transaction("0x07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30",
                                        "5d996aa76b3212142792d9130796cd2e11e3c445a93118c08414df4f66bc60ec",
                                        "transfer",
                                        99,
                                        33,
                                        3296766,
                                        3664390082,
                                        100,
                                        "",
                                        Some(OpsDetails::Transfer(Transfer {
                                            to: "0x07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30".to_string(),
                                            amount: 1000,
                                        })));
    let output = Signer::<StandardAptosContext>::sign_proto(input);
    test_tx_result(output,
                   "07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3063000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e74087472616e7366657200022007968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3008e803000000000000fe4d3200000000006400000000000000c2276ada0000000021",
                   "5707246db31e2335edc4316a7a656a11691d1d1647f6e864d1ab12f43428aaaf806cf02120d0b608cdd89c5c904af7b137432aacdd60cc53f9fad7bd33578e01",
                   "07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3063000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e74087472616e7366657200022007968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3008e803000000000000fe4d3200000000006400000000000000c2276ada00000000210020ea526ba1710343d953461ff68641f1b7df5f23b9042ffa2d2a798d3adb3f3d6c405707246db31e2335edc4316a7a656a11691d1d1647f6e864d1ab12f43428aaaf806cf02120d0b608cdd89c5c904af7b137432aacdd60cc53f9fad7bd33578e01",
                   r#"{
            "expiration_timestamp_secs": "3664390082",
            "gas_unit_price": "100",
            "max_gas_amount": "3296766",
            "payload": {
                "arguments": ["0x7968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30","1000"],
                "function": "0x1::aptos_account::transfer",
                "type": "entry_function_payload",
                "type_arguments": []
            },
            "sender": "0x7968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30",
            "sequence_number": "99",
            "signature": {
                "public_key": "0xea526ba1710343d953461ff68641f1b7df5f23b9042ffa2d2a798d3adb3f3d6c",
                "signature": "0x5707246db31e2335edc4316a7a656a11691d1d1647f6e864d1ab12f43428aaaf806cf02120d0b608cdd89c5c904af7b137432aacdd60cc53f9fad7bd33578e01",
                "type": "ed25519_signature"
            }
        }"#);
}

#[test]
fn test_aptos_sign_create_account() {
    let input = setup_proto_transaction("0x07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30", // Sender's address
                                        "5d996aa76b3212142792d9130796cd2e11e3c445a93118c08414df4f66bc60ec", // Keypair
                                        "create_account",
                                        0, // Sequence number
                                        33,
                                        3296766,
                                        3664390082,
                                        100,
                                        "",
                                        Some(OpsDetails::AccountCreation(AccountCreation {
                                            to: "0x3aa1672641a4e17b3d913b4c0301e805755a80b12756fc729c5878f12344d30e".to_string(),
                                        })));
    let output = Signer::<StandardAptosContext>::sign_proto(input);
    test_tx_result(output,
                   "07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3000000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e740e6372656174655f6163636f756e740001203aa1672641a4e17b3d913b4c0301e805755a80b12756fc729c5878f12344d30efe4d3200000000006400000000000000c2276ada0000000021", // Expected raw transaction bytes
                   "fcba3dfbec76721454ef414955f09f159660a13886b4edd8c579e3c779c29073afe7b25efa3fef9b21c2efb1cf16b4247fc0e5c8f63fdcd1c8d87f5d59f44501", // Expected signature
                   "07968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f3000000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e740e6372656174655f6163636f756e740001203aa1672641a4e17b3d913b4c0301e805755a80b12756fc729c5878f12344d30efe4d3200000000006400000000000000c2276ada00000000210020ea526ba1710343d953461ff68641f1b7df5f23b9042ffa2d2a798d3adb3f3d6c40fcba3dfbec76721454ef414955f09f159660a13886b4edd8c579e3c779c29073afe7b25efa3fef9b21c2efb1cf16b4247fc0e5c8f63fdcd1c8d87f5d59f44501", // Expected encoded transaction
                   r#"{
            "expiration_timestamp_secs": "3664390082",
            "gas_unit_price": "100",
            "max_gas_amount": "3296766",
            "payload": {
                "arguments": ["0x3aa1672641a4e17b3d913b4c0301e805755a80b12756fc729c5878f12344d30e"],
                "function": "0x1::aptos_account::create_account",
                "type": "entry_function_payload",
                "type_arguments": []
            },
            "sender": "0x7968dab936c1bad187c60ce4082f307d030d780e91e694ae03aef16aba73f30",
            "sequence_number": "0",
            "signature": {
                "public_key": "0xea526ba1710343d953461ff68641f1b7df5f23b9042ffa2d2a798d3adb3f3d6c",
                "signature": "0xfcba3dfbec76721454ef414955f09f159660a13886b4edd8c579e3c779c29073afe7b25efa3fef9b21c2efb1cf16b4247fc0e5c8f63fdcd1c8d87f5d59f44501",
                "type": "ed25519_signature"
            }
        }"#);
}