// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::context::CosmosContext;
use crate::proto::cosmos::base::v1beta1 as base_proto;
use crate::proto::cosmos::signing::v1beta1 as signing_proto;
use crate::proto::cosmos::tx::v1beta1 as tx_proto;
use crate::public_key::ProtobufPublicKey;
use crate::transaction::{
    Coin, Fee, SignMode, SignedTransaction, SignerInfo, TxBody, UnsignedTransaction,
};
use std::marker::PhantomData;
use tw_proto::serialize;

pub fn build_coin(coin: &Coin) -> base_proto::Coin {
    base_proto::Coin {
        denom: coin.denom.clone(),
        amount: coin.amount.to_string(),
    }
}

/// `ProtobufSerializer` serializes Cosmos specific Protobuf messages.
pub struct ProtobufSerializer<Context> {
    _phantom: PhantomData<Context>,
}

impl<Context> ProtobufSerializer<Context>
where
    Context: CosmosContext,
    Context::PublicKey: ProtobufPublicKey,
{
    /// Serializes a signed transaction into the Cosmos [`tx_proto::TxRaw`] message.
    /// [`tx_proto::TxRaw`] can be broadcasted to the network.
    /// TODO rename
    pub fn build_signed_tx(signed: &SignedTransaction<Context>) -> tx_proto::TxRaw {
        let tx_body = Self::build_tx_body(&signed.tx_body);
        let body_bytes = serialize(&tx_body).expect("Unexpected error on tx_body serialization");

        let auth_info = Self::build_auth_info(&signed.signer, &signed.fee);
        let auth_info_bytes =
            serialize(&auth_info).expect("Unexpected error on auth_info serialization");

        tx_proto::TxRaw {
            body_bytes,
            auth_info_bytes,
            signatures: vec![signed.signature.clone()],
        }
    }

    /// Serializes an unsigned transaction into the Cosmos [`tx_proto::SignDoc`] message.
    /// [`tx_proto::SignDoc`] is used to generate a transaction prehash and sign it.
    pub fn build_sign_doc(unsigned: &UnsignedTransaction<Context>) -> tx_proto::SignDoc {
        let tx_body = Self::build_tx_body(&unsigned.tx_body);
        let body_bytes = serialize(&tx_body).expect("Unexpected error on tx_body serialization");

        let auth_info = Self::build_auth_info(&unsigned.signer, &unsigned.fee);
        let auth_info_bytes =
            serialize(&auth_info).expect("Unexpected error on auth_info serialization");

        tx_proto::SignDoc {
            body_bytes,
            auth_info_bytes,
            chain_id: unsigned.chain_id.clone(),
            account_number: unsigned.account_number,
        }
    }

    pub fn build_auth_info(
        signer: &SignerInfo<Context::PublicKey>,
        fee: &Fee<Context::Address>,
    ) -> tx_proto::AuthInfo {
        tx_proto::AuthInfo {
            signer_infos: vec![Self::build_signer_info(signer)],
            fee: Some(Self::build_fee(fee)),
            // At this moment, we do not support transaction tip.
            tip: None,
        }
    }

    pub fn build_tx_body(tx_body: &TxBody) -> tx_proto::TxBody {
        let messages: Vec<_> = tx_body.messages.iter().map(|msg| msg.to_proto()).collect();
        tx_proto::TxBody {
            messages,
            memo: tx_body.memo.clone(),
            timeout_height: tx_body.timeout_height,
            extension_options: Vec::default(),
            non_critical_extension_options: Vec::default(),
        }
    }

    pub fn build_signer_info(signer: &SignerInfo<Context::PublicKey>) -> tx_proto::SignerInfo {
        use tx_proto::mod_ModeInfo::{self as mode_info, OneOfsum as SumEnum};

        // Single is the mode info for a single signer. It is structured as a message
        // to allow for additional fields such as locale for SIGN_MODE_TEXTUAL in the future.
        let mode_info = tx_proto::ModeInfo {
            sum: SumEnum::single(mode_info::Single {
                mode: Self::build_sign_mode(signer.sign_mode),
            }),
        };

        tx_proto::SignerInfo {
            public_key: Some(signer.public_key.to_proto()),
            mode_info: Some(mode_info),
            sequence: signer.sequence,
        }
    }

    fn build_fee(fee: &Fee<Context::Address>) -> tx_proto::Fee {
        let payer = fee
            .payer
            .as_ref()
            .map(Context::Address::to_string)
            .unwrap_or_default();
        let granter = fee
            .granter
            .as_ref()
            .map(Context::Address::to_string)
            .unwrap_or_default();

        tx_proto::Fee {
            amount: fee.amounts.iter().map(build_coin).collect(),
            gas_limit: fee.gas_limit,
            payer,
            granter,
        }
    }

    fn build_sign_mode(sign_mode: SignMode) -> signing_proto::SignMode {
        match sign_mode {
            SignMode::Direct => signing_proto::SignMode::SIGN_MODE_DIRECT,
        }
    }
}
