// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::address::BinanceAddress;
use crate::amino::Amino;
use crate::transaction::message::{message_to_json, BinanceMessage};
use serde::Serialize;
use serde_json::Value as Json;
use tw_coin_entry::coin_entry::CoinAddress;
use tw_coin_entry::error::{SigningError, SigningResult};
use tw_memory::Data;
use tw_proto::Binance::Proto;

#[repr(i64)]
#[derive(Clone, Copy, serde_repr::Serialize_repr, strum_macros::FromRepr)]
pub enum OrderType {
    /// https://github.com/bnb-chain/python-sdk/blob/0f6b8a6077f486b26eda3e448f3e84ef35bfff75/binance_chain/constants.py#L62
    Limit = 2,
}

#[derive(Serialize)]
pub struct NewTradeOrder {
    /// Order id, optional.
    pub id: String,
    /// Order type.
    #[serde(rename = "ordertype")]
    pub order_type: OrderType,
    /// Price of the order, which is the real price multiplied by 1e8 (10^8) and rounded to integer.
    pub price: i64,
    /// Quantity of the order, which is the real price multiplied by 1e8 (10^8) and rounded to integer.
    pub quantity: i64,
    /// Originating address.
    pub sender: BinanceAddress,
    /// 1 for buy and 2 for sell.
    pub side: i64,
    /// Symbol for trading pair in full name of the tokens.
    pub symbol: String,
    /// 1 for Good Till Expire(GTE) order and 3 for Immediate Or Cancel (IOC).
    #[serde(rename = "timeinforce")]
    pub time_in_force: i64,
}

impl NewTradeOrder {
    pub const PREFIX: [u8; 4] = [0xCE, 0x6D, 0xC0, 0x43];
}

impl BinanceMessage for NewTradeOrder {
    fn to_json(&self) -> SigningResult<Json> {
        message_to_json(self)
    }

    fn to_amino_protobuf(&self) -> SigningResult<Data> {
        let msg = Proto::TradeOrder {
            id: self.id.clone().into(),
            ordertype: self.order_type as i64,
            price: self.price,
            quantity: self.quantity,
            sender: self.sender.data().into(),
            side: self.side,
            symbol: self.symbol.clone().into(),
            timeinforce: self.time_in_force,
        };
        Amino::encode_proto(&Self::PREFIX, &msg).map_err(SigningError::from)
    }
}
