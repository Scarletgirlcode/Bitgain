// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::bcs_encoding;
use crate::transaction_payload::{EntryFunction, TransactionPayload};
use move_core_types::{account_address::AccountAddress, ident_str, language_storage::ModuleId};
use serde_json::json;
use std::str::FromStr;
use tw_coin_entry::error::{SigningError, SigningErrorType, SigningResult};
use tw_proto::{
    Aptos::Proto::mod_LiquidStaking::OneOfliquid_stake_transaction_payload,
    Aptos::Proto::{LiquidStaking, TortugaClaim, TortugaStake, TortugaUnstake},
};

pub fn tortuga_stake(
    smart_contract_address: AccountAddress,
    amount: u64,
) -> SigningResult<TransactionPayload> {
    Ok(TransactionPayload::EntryFunction(EntryFunction::new(
        ModuleId::new(
            smart_contract_address,
            ident_str!("stake_router").to_owned(),
        ),
        ident_str!("stake").to_owned(),
        vec![],
        vec![bcs_encoding::encode(&amount)?],
        json!([amount.to_string()]),
    )))
}

pub fn tortuga_unstake(
    smart_contract_address: AccountAddress,
    amount: u64,
) -> SigningResult<TransactionPayload> {
    Ok(TransactionPayload::EntryFunction(EntryFunction::new(
        ModuleId::new(
            smart_contract_address,
            ident_str!("stake_router").to_owned(),
        ),
        ident_str!("unstake").to_owned(),
        vec![],
        vec![bcs_encoding::encode(&amount)?],
        json!([amount.to_string()]),
    )))
}

pub fn tortuga_claim(
    smart_contract_address: AccountAddress,
    idx: u64,
) -> SigningResult<TransactionPayload> {
    Ok(TransactionPayload::EntryFunction(EntryFunction::new(
        ModuleId::new(
            smart_contract_address,
            ident_str!("stake_router").to_owned(),
        ),
        ident_str!("claim").to_owned(),
        vec![],
        vec![bcs_encoding::encode(&idx)?],
        json!([idx.to_string()]),
    )))
}

pub struct Stake {
    pub amount: u64,
    pub smart_contract_address: AccountAddress,
}

pub struct Unstake {
    pub amount: u64,
    pub smart_contract_address: AccountAddress,
}

pub struct Claim {
    pub idx: u64,
    pub smart_contract_address: AccountAddress,
}

pub enum LiquidStakingOperation {
    Stake(Stake),
    Unstake(Unstake),
    Claim(Claim),
}

impl TryFrom<LiquidStaking<'_>> for LiquidStakingOperation {
    type Error = SigningError;

    fn try_from(value: LiquidStaking) -> SigningResult<Self> {
        match value.liquid_stake_transaction_payload {
            OneOfliquid_stake_transaction_payload::stake(stake_msg) => {
                let smart_contract_address =
                    AccountAddress::from_str(&value.smart_contract_address)
                        .map_err(|_| SigningError(SigningErrorType::Error_invalid_address))?;
                Ok(LiquidStakingOperation::Stake(Stake {
                    amount: stake_msg.amount,
                    smart_contract_address,
                }))
            },
            OneOfliquid_stake_transaction_payload::unstake(unstake_msg) => {
                let smart_contract_address =
                    AccountAddress::from_str(&value.smart_contract_address)
                        .map_err(|_| SigningError(SigningErrorType::Error_invalid_address))?;
                Ok(LiquidStakingOperation::Unstake(Unstake {
                    amount: unstake_msg.amount,
                    smart_contract_address,
                }))
            },
            OneOfliquid_stake_transaction_payload::claim(claim) => {
                let smart_contract_address =
                    AccountAddress::from_str(&value.smart_contract_address)
                        .map_err(|_| SigningError(SigningErrorType::Error_invalid_address))?;
                Ok(LiquidStakingOperation::Claim(Claim {
                    idx: claim.idx,
                    smart_contract_address,
                }))
            },
            OneOfliquid_stake_transaction_payload::None => {
                Err(SigningError(SigningErrorType::Error_invalid_params))
            },
        }
    }
}

impl From<LiquidStakingOperation> for LiquidStaking<'_> {
    fn from(value: LiquidStakingOperation) -> Self {
        match value {
            LiquidStakingOperation::Stake(stake) => LiquidStaking {
                smart_contract_address: stake.smart_contract_address.to_hex_literal().into(),
                liquid_stake_transaction_payload: OneOfliquid_stake_transaction_payload::stake(
                    TortugaStake {
                        amount: stake.amount,
                    },
                ),
            },
            LiquidStakingOperation::Unstake(unstake) => LiquidStaking {
                smart_contract_address: unstake.smart_contract_address.to_hex_literal().into(),
                liquid_stake_transaction_payload: OneOfliquid_stake_transaction_payload::unstake(
                    TortugaUnstake {
                        amount: unstake.amount,
                    },
                ),
            },
            LiquidStakingOperation::Claim(claim) => LiquidStaking {
                smart_contract_address: claim.smart_contract_address.to_hex_literal().into(),
                liquid_stake_transaction_payload: OneOfliquid_stake_transaction_payload::claim(
                    TortugaClaim { idx: claim.idx },
                ),
            },
        }
    }
}
