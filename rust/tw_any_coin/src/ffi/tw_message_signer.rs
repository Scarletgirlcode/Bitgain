// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#![allow(clippy::missing_safety_doc)]

use crate::message_signer::MessageSigner;
use tw_memory::ffi::tw_data::TWData;
use tw_memory::ffi::RawPtrTrait;
use tw_misc::{try_or_else, try_or_false};

/// Signs a message for the given blockchain.
///
/// \param input The serialized data of a signing input (e.g. TW.Ethereum.Proto.MessageSigningInput).
/// \param coin The given coin type to sign the transaction for.
/// \return The serialized data of a `SigningOutput` proto object. (e.g. TW.Ethereum.Proto.MessageSigningOutput).
#[no_mangle]
pub unsafe extern "C" fn tw_message_signer_sign(input: *const TWData, coin: u32) -> *mut TWData {
    let input = try_or_else!(TWData::from_ptr_as_ref(input), std::ptr::null_mut);

    MessageSigner::sign_message(input.as_slice(), coin)
        .map(|output| TWData::from(output).into_ptr())
        .unwrap_or_else(|_| std::ptr::null_mut())
}

/// Verifies a signature for a message.
///
/// \param input The serialized data of a signing input (e.g. TW.Ethereum.Proto.MessageSigningInput).
/// \param coin The given coin type to sign the transaction for.
/// \return The serialized data of a `SigningOutput` proto object. (e.g. TW.Ethereum.Proto.MessageSigningOutput).
#[no_mangle]
pub unsafe extern "C" fn tw_message_signer_verify(input: *const TWData, coin: u32) -> bool {
    let input = try_or_false!(TWData::from_ptr_as_ref(input));
    MessageSigner::verify_message(input.as_slice(), coin).unwrap_or_default()
}
