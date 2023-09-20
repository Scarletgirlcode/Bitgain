// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#![allow(clippy::missing_safety_doc)]

use tw_coin_registry::coin_type::CoinType;
use tw_coin_registry::dispatcher::evm_dispatcher;
use tw_memory::ffi::tw_data::TWData;
use tw_memory::ffi::RawPtrTrait;
use tw_misc::try_or_else;

/// Decode function call data to human readable json format, according to input abi json.
///
/// \param coin EVM-compatible coin type.
/// \param data Non-null block of data
/// \return serialized `EthereumAbi::Proto::DecodeContractCallInput`.
#[no_mangle]
pub unsafe extern "C" fn tw_ethereum_abi_decode_contract_call(
    coin: CoinType,
    input: *const TWData,
) -> *mut TWData {
    let input_data = try_or_else!(TWData::from_ptr_as_ref(input), std::ptr::null_mut);
    let evm_dispatcher = try_or_else!(evm_dispatcher(coin), std::ptr::null_mut);

    evm_dispatcher
        .decode_contract_call(input_data.as_slice())
        .map(|data| TWData::from(data).into_ptr())
        .unwrap_or_else(|_| std::ptr::null_mut())
}
