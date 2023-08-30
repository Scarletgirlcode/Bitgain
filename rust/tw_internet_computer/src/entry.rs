// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use tw_coin_entry::{
    coin_context::CoinContext,
    coin_entry::CoinEntry,
    error::{AddressError, AddressResult},
    modules::{json_signer::NoJsonSigner, plan_builder::NoPlanBuilder},
    prefix::NoPrefix,
};

use tw_proto::InternetComputer::Proto;
use tw_proto::TxCompiler::Proto as CompilerProto;

use crate::{
    context::StandardInternetComputerContext, icp::address::AccountIdentifier,
    principal::Principal, signer::Signer,
};

pub struct InternetComputerEntry;

impl CoinEntry for InternetComputerEntry {
    type AddressPrefix = NoPrefix;

    type Address = AccountIdentifier;

    type SigningInput<'a> = Proto::SigningInput<'a>;

    type SigningOutput = Proto::SigningOutput<'static>;

    type PreSigningOutput = CompilerProto::PreSigningOutput<'static>;

    type JsonSigner = NoJsonSigner;

    type PlanBuilder = NoPlanBuilder;

    #[inline]
    fn parse_address(
        &self,
        _coin: &dyn CoinContext,
        address: &str,
        _prefix: Option<Self::AddressPrefix>,
    ) -> AddressResult<Self::Address> {
        AccountIdentifier::from_hex(address).map_err(|_| AddressError::FromHexError)
    }

    #[inline]
    fn derive_address(
        &self,
        _coin: &dyn tw_coin_entry::coin_context::CoinContext,
        public_key: tw_keypair::tw::PublicKey,
        _derivation: tw_coin_entry::derivation::Derivation,
        _prefix: Option<Self::AddressPrefix>,
    ) -> tw_coin_entry::error::AddressResult<Self::Address> {
        let secp256k1_public_key = public_key
            .to_secp256k1()
            .ok_or(AddressError::PublicKeyTypeMismatch)?;
        let principal = Principal::from(secp256k1_public_key);
        let address = AccountIdentifier::new(&principal);
        Ok(address)
    }

    #[inline]
    fn sign(
        &self,
        _coin: &dyn tw_coin_entry::coin_context::CoinContext,
        input: Self::SigningInput<'_>,
    ) -> Self::SigningOutput {
        Signer::<StandardInternetComputerContext>::sign_proto(input)
    }

    fn preimage_hashes(
        &self,
        _coin: &dyn tw_coin_entry::coin_context::CoinContext,
        _input: Self::SigningInput<'_>,
    ) -> Self::PreSigningOutput {
        unimplemented!()
    }

    fn compile(
        &self,
        _coin: &dyn tw_coin_entry::coin_context::CoinContext,
        _input: Self::SigningInput<'_>,
        _signatures: Vec<tw_coin_entry::coin_entry::SignatureBytes>,
        _public_keys: Vec<tw_coin_entry::coin_entry::PublicKeyBytes>,
    ) -> Self::SigningOutput {
        unimplemented!()
    }
}
