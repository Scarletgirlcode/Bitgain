// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::modules::tx_builder::TxBuilder;
use crate::modules::wallet_connect::types::SignAminoRequest;
use tw_coin_entry::coin_context::CoinContext;
use tw_coin_entry::error::{SigningError, SigningErrorType, SigningResult};
use tw_coin_entry::modules::wallet_connect_signer::WalletConnector;
use tw_coin_entry::signing_output_error;
use tw_proto::WalletConnect::Proto::{
    self as WCProto, mod_ParseRequestOutput::OneOfsigning_input_oneof as SigningInputEnum,
};

pub struct BinanceWalletConnector;

impl WalletConnector for BinanceWalletConnector {
    fn parse_request(
        &self,
        coin: &dyn CoinContext,
        request: WCProto::ParseRequestInput<'_>,
    ) -> WCProto::ParseRequestOutput<'static> {
        Self::parse_request_impl(coin, request)
            .unwrap_or_else(|e| signing_output_error!(WCProto::ParseRequestOutput, e))
    }
}

impl BinanceWalletConnector {
    fn parse_request_impl(
        coin: &dyn CoinContext,
        request: WCProto::ParseRequestInput<'_>,
    ) -> SigningResult<WCProto::ParseRequestOutput<'static>> {
        match request.method {
            WCProto::Method::CosmosSignAmino => Self::parse_sign_amino_request(coin, request),
            _ => Err(SigningError(SigningErrorType::Error_not_supported)),
        }
    }

    pub fn parse_sign_amino_request(
        _coin: &dyn CoinContext,
        request: WCProto::ParseRequestInput<'_>,
    ) -> SigningResult<WCProto::ParseRequestOutput<'static>> {
        let amino_req: SignAminoRequest = serde_json::from_str(&request.payload)
            .map_err(|_| SigningError(SigningErrorType::Error_input_parse))?;

        // Parse a `SigningInput` from the given `signDoc`.
        let signing_input = TxBuilder::unsigned_tx_to_proto(&amino_req.sign_doc)?;

        Ok(WCProto::ParseRequestOutput {
            signing_input_oneof: SigningInputEnum::binance(signing_input),
            ..WCProto::ParseRequestOutput::default()
        })
    }
}
