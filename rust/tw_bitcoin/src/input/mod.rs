use crate::InputContext;
use bitcoin::{ScriptBuf, TxIn, Witness};

mod p2pkh;
mod p2tr_key_path;
mod p2tr_script_path;
mod p2wpkh;

pub use p2pkh::*;
pub use p2tr_key_path::*;
pub use p2tr_script_path::*;
pub use p2wpkh::*;

#[derive(Debug, Clone)]
pub enum TxInput {
    P2PKH(TxInputP2PKH),
    P2WPKH(TxInputP2WPKH),
    P2TRKeyPath(TxInputP2TRKeyPath),
    P2TRScriptPath(TxInputP2TRScriptPath),
    NonStandard { ctx: InputContext },
}

impl From<TxInputP2PKH> for TxInput {
    fn from(input: TxInputP2PKH) -> Self {
        TxInput::P2PKH(input)
    }
}

impl From<TxInputP2WPKH> for TxInput {
    fn from(input: TxInputP2WPKH) -> Self {
        TxInput::P2WPKH(input)
    }
}

impl From<TxInputP2TRKeyPath> for TxInput {
    fn from(input: TxInputP2TRKeyPath) -> Self {
        TxInput::P2TRKeyPath(input)
    }
}

impl From<TxInputP2TRScriptPath> for TxInput {
    fn from(input: TxInputP2TRScriptPath) -> Self {
        TxInput::P2TRScriptPath(input)
    }
}

impl From<TxInput> for TxIn {
    fn from(input: TxInput) -> Self {
        let ctx = input.ctx();

        TxIn {
            previous_output: ctx.previous_output,
            script_sig: ScriptBuf::new(),
            sequence: ctx.sequence,
            witness: Witness::default(),
        }
    }
}

impl TxInput {
    pub fn ctx(&self) -> &InputContext {
        match self {
            TxInput::P2PKH(t) => &t.ctx,
            TxInput::P2WPKH(t) => &t.ctx,
            TxInput::P2TRKeyPath(t) => &t.ctx,
            TxInput::P2TRScriptPath(t) => &t.ctx,
            TxInput::NonStandard { ctx } => ctx,
        }
    }
    pub fn satoshis(&self) -> Option<u64> {
        match self {
            TxInput::P2PKH(t) => t.ctx.value,
            TxInput::P2WPKH(t) => t.ctx.value,
            TxInput::P2TRKeyPath(t) => t.ctx.value,
            TxInput::P2TRScriptPath(t) => t.ctx.value,
            TxInput::NonStandard { ctx } => ctx.value,
        }
    }
}
