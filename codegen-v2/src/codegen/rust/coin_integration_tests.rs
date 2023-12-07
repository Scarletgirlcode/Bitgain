// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::codegen::rust::tw_any_coin_directory;
use crate::codegen::template_generator::TemplateGenerator;
use crate::coin_id::CoinId;
use crate::registry::CoinItem;
use crate::utils::FileContent;
use crate::Result;
use std::fs;
use std::path::PathBuf;

const ADDRESS_TESTS_TEMPLATE: &str = include_str!("templates/integration_tests/address_tests.rs");
const COMPILE_TESTS_TEMPLATE: &str = include_str!("templates/integration_tests/compile_tests.rs");
const MOD_TESTS_TEMPLATE: &str = include_str!("templates/integration_tests/mod.rs");
const SIGN_TESTS_TEMPLATE: &str = include_str!("templates/integration_tests/sign_tests.rs");

pub fn chains_integration_tests_directory() -> PathBuf {
    tw_any_coin_directory().join("tests").join("chains")
}

pub fn coin_integration_tests_directory(id: &CoinId) -> PathBuf {
    chains_integration_tests_directory().join(id.as_str())
}

pub fn coin_address_derivation_test_path() -> PathBuf {
    tw_any_coin_directory()
        .join("tests")
        .join("coin_address_derivation_test.rs")
}

pub struct CoinIntegrationTests {
    coin: CoinItem,
}

impl CoinIntegrationTests {
    pub fn new(coin: CoinItem) -> CoinIntegrationTests {
        CoinIntegrationTests { coin }
    }

    pub fn create(self) -> Result<PathBuf> {
        let blockchain_tests_path = self.coin_tests_directory();
        if blockchain_tests_path.exists() {
            println!("[SKIP] integration tests already exists: {blockchain_tests_path:?}");
            return Ok(blockchain_tests_path);
        }

        fs::create_dir_all(&blockchain_tests_path)?;

        self.list_blockchain_in_chains_mod()?;
        self.create_address_tests()?;
        self.create_compile_tests()?;
        self.create_sign_tests()?;
        self.create_chain_tests_mod_rs()?;

        Ok(blockchain_tests_path)
    }

    fn coin_tests_directory(&self) -> PathBuf {
        coin_integration_tests_directory(&self.coin.id)
    }

    fn create_address_tests(&self) -> Result<()> {
        let coin_id = self.coin.id.as_str();
        let address_tests_path = self
            .coin_tests_directory()
            .join(format!("{coin_id}_address.rs"));

        println!("[ADD] {address_tests_path:?}");
        TemplateGenerator::new(ADDRESS_TESTS_TEMPLATE)
            .write_to(address_tests_path)
            .with_default_patterns(&self.coin)
            .write()
    }

    fn create_compile_tests(&self) -> Result<()> {
        let coin_id = self.coin.id.as_str();
        let compile_tests_path = self
            .coin_tests_directory()
            .join(format!("{coin_id}_compile.rs"));

        println!("[ADD] {compile_tests_path:?}");
        TemplateGenerator::new(COMPILE_TESTS_TEMPLATE)
            .write_to(compile_tests_path)
            .with_default_patterns(&self.coin)
            .write()
    }

    fn create_sign_tests(&self) -> Result<()> {
        let coin_id = self.coin.id.as_str();
        let sign_tests_path = self
            .coin_tests_directory()
            .join(format!("{coin_id}_sign.rs"));

        println!("[ADD] {sign_tests_path:?}");
        TemplateGenerator::new(SIGN_TESTS_TEMPLATE)
            .write_to(sign_tests_path)
            .with_default_patterns(&self.coin)
            .write()
    }

    fn create_chain_tests_mod_rs(&self) -> Result<()> {
        let blockchain_tests_mod_path = self.coin_tests_directory().join("mod.rs");

        println!("[ADD] {blockchain_tests_mod_path:?}");
        TemplateGenerator::new(MOD_TESTS_TEMPLATE)
            .write_to(blockchain_tests_mod_path)
            .with_default_patterns(&self.coin)
            .write()
    }

    fn list_blockchain_in_chains_mod(&self) -> Result<()> {
        let chains_mod_path = chains_integration_tests_directory().join("mod.rs");
        let chain_id = self.coin.id.as_str();

        println!("[EDIT] {chains_mod_path:?}");
        let mut chains_mod_rs = FileContent::read(chains_mod_path)?;

        {
            let mod_pattern = "mod ";
            let mut mod_region = chains_mod_rs.find_region_with_prefix(mod_pattern)?;
            mod_region.push_line(format!("mod {chain_id};"));
            mod_region.sort();
        }

        chains_mod_rs.write()
    }
}
