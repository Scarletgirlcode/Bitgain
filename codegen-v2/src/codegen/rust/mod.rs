// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use std::env;
use std::path::PathBuf;

pub mod blockchain_dispatcher_generator;
pub mod blockchain_type_generator;
pub mod coin_address_derivation_test_generator;
pub mod coin_crate;
pub mod coin_integration_tests;
pub mod coin_registry_manifest_generator;
pub mod new_blockchain;
pub mod new_evmchain;
pub mod toml_editor;

pub fn rust_source_directory() -> PathBuf {
    PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .join("..")
        .join("rust")
}

pub fn chains_directory() -> PathBuf {
    rust_source_directory().join("chains")
}

pub fn tw_any_coin_directory() -> PathBuf {
    rust_source_directory().join("tw_any_coin")
}

pub fn workspace_toml_path() -> PathBuf {
    rust_source_directory().join("Cargo.toml")
}

pub fn coin_registry_directory() -> PathBuf {
    rust_source_directory().join("tw_coin_registry")
}
