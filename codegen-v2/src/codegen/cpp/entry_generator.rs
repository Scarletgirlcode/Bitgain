// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::codegen::cpp::cpp_source_directory;
use crate::codegen::template_generator::TemplateGenerator;
use crate::registry::CoinItem;
use crate::Result;
use std::fs;
use std::path::PathBuf;

const ENTRY_HEADER_TEMPLATE: &str = include_str!("templates/Entry.h");

pub fn coin_source_directory(coin: &CoinItem) -> PathBuf {
    cpp_source_directory().join(coin.blockchain_type())
}

pub struct EntryGenerator;

impl EntryGenerator {
    pub fn generate(coin: &CoinItem) -> Result<PathBuf> {
        let blockchain_dir = coin_source_directory(coin);
        let entry_header_path = blockchain_dir.join("Entry.h");

        if blockchain_dir.exists() {
            println!("[SKIP] Entry file already exists: {blockchain_dir:?}");
            return Ok(blockchain_dir);
        }

        fs::create_dir_all(&blockchain_dir)?;

        println!("[ADD] {entry_header_path:?}");
        TemplateGenerator::new(ENTRY_HEADER_TEMPLATE)
            .write_to(entry_header_path.clone())
            .with_default_patterns(coin)
            .write()?;

        Ok(entry_header_path)
    }
}
