// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use serde_json::{json, Value as Json};
use std::borrow::Cow;
use tw_coin_entry::error::SigningErrorType;
use tw_encoding::hex::{DecodeHex, ToHex};
use tw_evm::evm_context::StandardEvmContext;
use tw_evm::modules::abi_encoder::AbiEncoder;
use tw_number::U256;
use tw_proto::EthereumAbi::Proto;

use Proto::mod_ParamType::OneOfparam as ParamTypeEnum;
use Proto::mod_ParamValue::OneOfparam as ParamEnum;

fn named_param_type(name: &str, kind: ParamTypeEnum<'static>) -> Proto::NamedParamType<'static> {
    Proto::NamedParamType {
        name: name.to_string().into(),
        param: Some(Proto::ParamType { param: kind }),
    }
}

fn named_param(name: &str, value: ParamEnum<'static>) -> Proto::NamedParam<'static> {
    Proto::NamedParam {
        name: Cow::Owned(name.to_string()),
        value: Some(Proto::ParamValue { param: value }),
    }
}

fn number_n<const BITS: u32>(value: u64) -> Proto::NumberNParam<'static> {
    Proto::NumberNParam {
        bits: BITS,
        value: U256::encode_be_compact(value),
    }
}

fn number_type_n<const BITS: u32>() -> Proto::NumberNType {
    Proto::NumberNType { bits: BITS }
}

fn array(
    element_type: ParamTypeEnum<'static>,
    values: Vec<ParamEnum<'static>>,
) -> Proto::ArrayParam<'static> {
    let values = values
        .into_iter()
        .map(|param| Proto::ParamValue { param })
        .collect();
    let element_type = Some(Proto::ParamType {
        param: element_type,
    });
    Proto::ArrayParam {
        element_type,
        values,
    }
}

fn tuple<I>(params: I) -> ParamEnum<'static>
where
    I: IntoIterator<Item = Proto::NamedParam<'static>>,
{
    ParamEnum::tuple(Proto::TupleParam {
        params: params.into_iter().collect(),
    })
}

#[test]
fn test_decode_contract_call() {
    const SWAP_V2_ABI: &str = include_str!("data/swap_v2.json");
    const SWAP_V2_DECODED: &str = include_str!("data/swap_v2_decoded.json");

    let encoded_input = "846a1bc6000000000000000000000000dac17f958d2ee523a2206206994597c13d831ec700000000000000000000000000000000000000000000000000470de4df82000000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000740000000000000000000000000000000000000000000000000000000000000078000000000000000000000000000000000000000000000000000000000000007c00000000000000000000000000000000000000000000000000000000000000820000000000000000000000000a140f413c63fbda84e9008607e678258fffbc76b00000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000001800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000dac17f958d2ee523a2206206994597c13d831ec7000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000044095ea7b300000000000000000000000099a58482bd75cbab83b27ec03ca68ff489b5788f00000000000000000000000000000000000000000000000000470de4df820000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099a58482bd75cbab83b27ec03ca68ff489b5788f000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000046000000000000000000000000000000000000000000000000000000000000003840651cb35000000000000000000000000dac17f958d2ee523a2206206994597c13d831ec7000000000000000000000000bebc44782c7db0a1a60cb6fe97d0b483032ff1c7000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb4800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000470de4df8200000000000000000000000000000000000000000000000000000000298ce42936ed0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ce16f69375520ab01377ce7b88f5ba8c48f8d66600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000045553444300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000762696e616e636500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002a307863653136463639333735353230616230313337376365374238386635424138433438463844363636000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000a140f413c63fbda84e9008607e678258fffbc76b000000000000000000000000000000000000000000000000000000000000000700000000000000000000000000000000000000000000000000000000000000e000000000000000000000000000000000000000000000000000000000000001e0000000000000000000000000000000000000000000000000000000000000036000000000000000000000000000000000000000000000000000000000000005a0000000000000000000000000000000000000000000000000000000000000072000000000000000000000000000000000000000000000000000000000000009600000000000000000000000000000000000000000000000000000000000000ac000000000000000000000000000000000000000000000000000000000000000030000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000200000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf300000000000000000000000000000000000000000000000000000000000000010000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf3000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000044095ea7b30000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb1400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf3000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001e00000000000000000000000000000000000000000000000000000000000000104414bf3890000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf300000000000000000000000055d398326f99059ff775485246999027b319795500000000000000000000000000000000000000000000000000000000000000640000000000000000000000004fd39c9e151e50580779bd04b1f7ecc310079fd300000000000000000000000000000000000000000000000000000189c04a7044000000000000000000000000000000000000000000000000000029a23529cf68000000000000000000000000000000000000000000005af4f3f913bd553d03b900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf30000000000000000000000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000100000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000044095ea7b30000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004000000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001e00000000000000000000000000000000000000000000000000000000000000104414bf38900000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c00000000000000000000000000000000000000000000000000000000000001f40000000000000000000000004fd39c9e151e50580779bd04b1f7ecc310079fd300000000000000000000000000000000000000000000000000000189c04a7045000000000000000000000000000000000000000000005b527785e694f805bdd300000000000000000000000000000000000000000000005f935a1fa5c4a6ec61000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004000000000000000000000000055d398326f99059ff775485246999027b319795500000000000000000000000000000000000000000000000000000000000000050000000000000000000000000000000000000000000000000000000000000001000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000242e1a7d4d0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000a140f413c63fbda84e9008607e678258fffbc76b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000".decode_hex().unwrap();
    let input = Proto::ContractCallDecodingInput {
        encoded: Cow::Owned(encoded_input),
        smart_contract_abi_json: Cow::Borrowed(SWAP_V2_ABI),
    };

    let output = AbiEncoder::<StandardEvmContext>::decode_contract_call(input);
    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());

    let actual_json: Json = serde_json::from_str(&output.decoded_json).unwrap();
    let expected_json: Json = serde_json::from_str(SWAP_V2_DECODED).unwrap();
    assert_eq!(actual_json, expected_json);

    let call_data_1 = "0x095ea7b300000000000000000000000099a58482bd75cbab83b27ec03ca68ff489b5788f00000000000000000000000000000000000000000000000000470de4df820000".decode_hex().unwrap();
    let call_data_2 = "0x0651cb35000000000000000000000000dac17f958d2ee523a2206206994597c13d831ec7000000000000000000000000bebc44782c7db0a1a60cb6fe97d0b483032ff1c7000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb4800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000470de4df8200000000000000000000000000000000000000000000000000000000298ce42936ed0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ce16f69375520ab01377ce7b88f5ba8c48f8d666".decode_hex().unwrap();

    let array_inner_type = ParamTypeEnum::tuple(Proto::TupleType {
        params: vec![
            named_param_type("callType", ParamTypeEnum::number_uint(number_type_n::<8>())),
            named_param_type("target", ParamTypeEnum::address(Proto::AddressType {})),
            named_param_type("value", ParamTypeEnum::number_uint(number_type_n::<256>())),
            named_param_type(
                "callData",
                ParamTypeEnum::byte_array(Proto::ByteArrayType {}),
            ),
            named_param_type(
                "payload",
                ParamTypeEnum::byte_array(Proto::ByteArrayType {}),
            ),
        ],
    });
    let expected_calls = vec![
        tuple(vec![
            named_param("callType", ParamEnum::number_uint(number_n::<8>(0))),
            named_param(
                "target",
                ParamEnum::address("0xdAC17F958D2ee523a2206206994597C13D831ec7".into()),
            ),
            named_param("value", ParamEnum::number_uint(number_n::<256>(0))),
            named_param("callData", ParamEnum::byte_array(call_data_1.into())),
            named_param("payload", ParamEnum::byte_array(Cow::default())),
        ]),
        tuple(vec![
            named_param("callType", ParamEnum::number_uint(number_n::<8>(0))),
            named_param(
                "target",
                ParamEnum::address("0x99a58482BD75cbab83b27EC03CA68fF489b5788f".into()),
            ),
            named_param("value", ParamEnum::number_uint(number_n::<256>(0))),
            named_param("callData", ParamEnum::byte_array(call_data_2.into())),
            named_param("payload", ParamEnum::byte_array(Cow::default())),
        ]),
    ];

    let payload = "0x0000000000000000000000000000000000000000000000000000000000000040000000000000000000000000a140f413c63fbda84e9008607e678258fffbc76b000000000000000000000000000000000000000000000000000000000000000700000000000000000000000000000000000000000000000000000000000000e000000000000000000000000000000000000000000000000000000000000001e0000000000000000000000000000000000000000000000000000000000000036000000000000000000000000000000000000000000000000000000000000005a0000000000000000000000000000000000000000000000000000000000000072000000000000000000000000000000000000000000000000000000000000009600000000000000000000000000000000000000000000000000000000000000ac000000000000000000000000000000000000000000000000000000000000000030000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000200000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf300000000000000000000000000000000000000000000000000000000000000010000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf3000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000044095ea7b30000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb1400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf3000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001e00000000000000000000000000000000000000000000000000000000000000104414bf3890000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf300000000000000000000000055d398326f99059ff775485246999027b319795500000000000000000000000000000000000000000000000000000000000000640000000000000000000000004fd39c9e151e50580779bd04b1f7ecc310079fd300000000000000000000000000000000000000000000000000000189c04a7044000000000000000000000000000000000000000000000000000029a23529cf68000000000000000000000000000000000000000000005af4f3f913bd553d03b900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000004268b8f0b87b6eae5d897996e6b845ddbd99adf30000000000000000000000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000100000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000000044095ea7b30000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004000000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000001b81d678ffb9c0263b24a97847620c99d213eb14000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000001e00000000000000000000000000000000000000000000000000000000000000104414bf38900000000000000000000000055d398326f99059ff775485246999027b3197955000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c00000000000000000000000000000000000000000000000000000000000001f40000000000000000000000004fd39c9e151e50580779bd04b1f7ecc310079fd300000000000000000000000000000000000000000000000000000189c04a7045000000000000000000000000000000000000000000005b527785e694f805bdd300000000000000000000000000000000000000000000005f935a1fa5c4a6ec61000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004000000000000000000000000055d398326f99059ff775485246999027b319795500000000000000000000000000000000000000000000000000000000000000050000000000000000000000000000000000000000000000000000000000000001000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000242e1a7d4d0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000bb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000a140f413c63fbda84e9008607e678258fffbc76b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000".decode_hex().unwrap();
    let expected_proto = vec![
        named_param(
            "token",
            ParamEnum::address("0xdAC17F958D2ee523a2206206994597C13D831ec7".into()),
        ),
        named_param(
            "amount",
            ParamEnum::number_uint(number_n::<256>(20000000000000000)),
        ),
        named_param(
            "calls",
            ParamEnum::array(array(array_inner_type, expected_calls)),
        ),
        named_param("bridgedTokenSymbol", ParamEnum::string_value("USDC".into())),
        named_param(
            "destinationChain",
            ParamEnum::string_value("binance".into()),
        ),
        named_param(
            "destinationAddress",
            ParamEnum::string_value("0xce16F69375520ab01377ce7B88f5BA8C48F8D666".into()),
        ),
        named_param("payload", ParamEnum::byte_array(payload.into())),
        named_param(
            "gasRefundRecipient",
            ParamEnum::address("0xa140F413C63FBDA84E9008607E678258ffFbC76b".into()),
        ),
        named_param("enableExpress", ParamEnum::boolean(true)),
    ];

    assert_eq!(output.params, expected_proto);
}

#[test]
fn test_decode_params_with_abi_json() {
    let abi_json = json!([
        {
            "internalType": "bytes32",
            "name": "node",
            "type": "bytes32"
        },
        {
            "internalType": "address",
            "name": "resolver",
            "type": "address"
        }
    ]);
    let abi_json = serde_json::to_string(&abi_json).unwrap();
    let encoded = "e71cd96d4ba1c4b512b0c5bee30d2b6becf61e574c32a17a67156fa9ed3c4c6f0000000000000000000000004976fb03c32e5b8cfe2b6ccb31c09ba78ebaba41".decode_hex().unwrap();

    let input = Proto::ParamsDecodingInput {
        encoded: Cow::Owned(encoded),
        abi: Proto::mod_ParamsDecodingInput::OneOfabi::abi_json(abi_json.into()),
    };

    let output = AbiEncoder::<StandardEvmContext>::decode_params(input);
    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());

    let node_bytes = "0xe71cd96d4ba1c4b512b0c5bee30d2b6becf61e574c32a17a67156fa9ed3c4c6f"
        .decode_hex()
        .unwrap();
    let expected_proto = vec![
        named_param("node", ParamEnum::byte_array_fix(node_bytes.into())),
        named_param(
            "resolver",
            ParamEnum::address("0x4976fb03C32e5B8cfe2b6cCB31c09Ba78EBaBa41".into()),
        ),
    ];
    assert_eq!(output.params, expected_proto);
}

#[test]
fn test_decode_params_with_abi_params() {
    let abi_params = vec![
        named_param_type(
            "node",
            ParamTypeEnum::byte_array_fix(Proto::ByteArrayFixType { size: 32 }),
        ),
        named_param_type("resolver", ParamTypeEnum::address(Proto::AddressType {})),
    ];
    let encoded = "e71cd96d4ba1c4b512b0c5bee30d2b6becf61e574c32a17a67156fa9ed3c4c6f0000000000000000000000004976fb03c32e5b8cfe2b6ccb31c09ba78ebaba41".decode_hex().unwrap();

    let input = Proto::ParamsDecodingInput {
        encoded: Cow::Owned(encoded),
        abi: Proto::mod_ParamsDecodingInput::OneOfabi::abi_params(Proto::AbiParams {
            params: abi_params,
        }),
    };

    let output = AbiEncoder::<StandardEvmContext>::decode_params(input);
    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());

    let node_bytes = "0xe71cd96d4ba1c4b512b0c5bee30d2b6becf61e574c32a17a67156fa9ed3c4c6f"
        .decode_hex()
        .unwrap();
    let expected_proto = vec![
        named_param("node", ParamEnum::byte_array_fix(node_bytes.into())),
        named_param(
            "resolver",
            ParamEnum::address("0x4976fb03C32e5B8cfe2b6cCB31c09Ba78EBaBa41".into()),
        ),
    ];
    assert_eq!(output.params, expected_proto);
}

#[test]
fn test_encode_params_monster() {
    let byte_array = "3132333435".decode_hex().unwrap();

    let u1 = ParamEnum::number_uint(number_n::<8>(1));
    let u2 = ParamEnum::number_uint(number_n::<16>(2));
    let u3 = ParamEnum::number_uint(number_n::<32>(3));
    let u4 = ParamEnum::number_uint(number_n::<64>(4));
    let u5 = ParamEnum::number_uint(number_n::<168>(0x123));
    let u6 = ParamEnum::number_uint(number_n::<256>(0x123));

    let u1t = ParamTypeEnum::number_uint(number_type_n::<8>());
    let u2t = ParamTypeEnum::number_uint(number_type_n::<16>());
    let u3t = ParamTypeEnum::number_uint(number_type_n::<32>());
    let u4t = ParamTypeEnum::number_uint(number_type_n::<64>());
    let u5t = ParamTypeEnum::number_uint(number_type_n::<168>());
    let u6t = ParamTypeEnum::number_uint(number_type_n::<256>());

    let i1 = ParamEnum::number_int(number_n::<8>(1));
    let i2 = ParamEnum::number_int(number_n::<16>(2));
    let i3 = ParamEnum::number_int(number_n::<32>(3));
    let i4 = ParamEnum::number_int(number_n::<64>(4));
    let i5 = ParamEnum::number_int(number_n::<168>(0x123));
    let i6 = ParamEnum::number_int(number_n::<256>(0x123));

    let i1t = ParamTypeEnum::number_int(number_type_n::<8>());
    let i2t = ParamTypeEnum::number_int(number_type_n::<16>());
    let i3t = ParamTypeEnum::number_int(number_type_n::<32>());
    let i4t = ParamTypeEnum::number_int(number_type_n::<64>());
    let i5t = ParamTypeEnum::number_int(number_type_n::<168>());
    let i6t = ParamTypeEnum::number_int(number_type_n::<256>());

    let b = ParamEnum::boolean(true);
    let bt = ParamTypeEnum::boolean(Proto::BoolType {});
    let s = ParamEnum::string_value("Hello, world!".into());
    let st = ParamTypeEnum::string_param(Proto::StringType {});
    let a = ParamEnum::address("0xf784682c82526e245f50975190ef0fff4e4fc077".into());
    let at = ParamTypeEnum::address(Proto::AddressType {});
    let bytes = ParamEnum::byte_array(byte_array.clone().into());
    let bytes_t = ParamTypeEnum::byte_array(Proto::ByteArrayType {});
    let fbytes = ParamEnum::byte_array_fix(byte_array.clone().into());
    let fbytes_t = ParamTypeEnum::byte_array_fix(Proto::ByteArrayFixType {
        size: byte_array.len() as u64,
    });

    let params = vec![
        // Uint
        named_param("u1", u1.clone()),
        named_param("u2", u2.clone()),
        named_param("u3", u3.clone()),
        named_param("u4", u4.clone()),
        named_param("u5", u5.clone()),
        named_param("u6", u6.clone()),
        // Int
        named_param("i1", i1.clone()),
        named_param("i2", i2.clone()),
        named_param("i3", i3.clone()),
        named_param("i4", i4.clone()),
        named_param("i5", i5.clone()),
        named_param("i6", i6.clone()),
        // Single params
        named_param("b", b.clone()),
        named_param("s", s.clone()),
        named_param("a", a.clone()),
        named_param("bytes", bytes.clone()),
        named_param("fbytes", fbytes.clone()),
        // Array<Uint>
        named_param("a_u1", ParamEnum::array(array(u1t, vec![u1]))),
        named_param("a_u2", ParamEnum::array(array(u2t, vec![u2]))),
        named_param("a_u3", ParamEnum::array(array(u3t, vec![u3]))),
        named_param("a_u4", ParamEnum::array(array(u4t, vec![u4]))),
        named_param("a_u5", ParamEnum::array(array(u5t, vec![u5]))),
        named_param("a_u6", ParamEnum::array(array(u6t, vec![u6]))),
        // Array<Int>
        named_param("a_i1", ParamEnum::array(array(i1t, vec![i1]))),
        named_param("a_i2", ParamEnum::array(array(i2t, vec![i2]))),
        named_param("a_i3", ParamEnum::array(array(i3t, vec![i3]))),
        named_param("a_i4", ParamEnum::array(array(i4t, vec![i4]))),
        named_param("a_i5", ParamEnum::array(array(i5t, vec![i5]))),
        named_param("a_i6", ParamEnum::array(array(i6t, vec![i6]))),
        // Arrays with single params
        named_param("a_b", ParamEnum::array(array(bt, vec![b]))),
        named_param("a_s", ParamEnum::array(array(st, vec![s]))),
        named_param("a_a", ParamEnum::array(array(at, vec![a]))),
        named_param("a_bytes", ParamEnum::array(array(bytes_t, vec![bytes]))),
        named_param("a_fbytes", ParamEnum::array(array(fbytes_t, vec![fbytes]))),
    ];
    let encoding_input = Proto::FunctionEncodingInput {
        function_name: "monster".into(),
        params,
    };
    let output = AbiEncoder::<StandardEvmContext>::encode_contract_call(encoding_input);

    assert_eq!(output.error, SigningErrorType::OK);
    assert!(output.error_message.is_empty());

    // let expected_encoded = "4061f075000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000002c0000000000000000000000000f784682c82526e245f50975190ef0fff4e4fc077000000000000000000000000000000000000000000000000000000000000030031323334350000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000340000000000000000000000000000000000000000000000000000000000000038000000000000000000000000000000000000000000000000000000000000003c000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000440000000000000000000000000000000000000000000000000000000000000048000000000000000000000000000000000000000000000000000000000000004c00000000000000000000000000000000000000000000000000000000000000500000000000000000000000000000000000000000000000000000000000000058000000000000000000000000000000000000000000000000000000000000005c00000000000000000000000000000000000000000000000000000000000000640000000000000000000000000000000000000000000000000000000000000000d48656c6c6f2c20776f726c642100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000531323334350000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000d48656c6c6f2c20776f726c6421000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000f784682c82526e245f50975190ef0fff4e4fc077000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000005313233343500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000013132333435000000000000000000000000000000000000000000000000000000";
    let expected_encoded = "70efb5a500000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000300000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000012300000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000300000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000012300000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000440000000000000000000000000f784682c82526e245f50975190ef0fff4e4fc0770000000000000000000000000000000000000000000000000000000000000480313233343500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004c000000000000000000000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000540000000000000000000000000000000000000000000000000000000000000058000000000000000000000000000000000000000000000000000000000000005c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000640000000000000000000000000000000000000000000000000000000000000068000000000000000000000000000000000000000000000000000000000000006c000000000000000000000000000000000000000000000000000000000000007000000000000000000000000000000000000000000000000000000000000000740000000000000000000000000000000000000000000000000000000000000078000000000000000000000000000000000000000000000000000000000000007c00000000000000000000000000000000000000000000000000000000000000800000000000000000000000000000000000000000000000000000000000000088000000000000000000000000000000000000000000000000000000000000008c00000000000000000000000000000000000000000000000000000000000000940000000000000000000000000000000000000000000000000000000000000000d48656c6c6f2c20776f726c642100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000531323334350000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000030000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000030000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000123000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001230000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000d48656c6c6f2c20776f726c6421000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000f784682c82526e245f50975190ef0fff4e4fc077000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000005313233343500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000013132333435000000000000000000000000000000000000000000000000000000";
    assert_eq!(output.encoded.to_hex(), expected_encoded);
}
