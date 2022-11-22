// Copyright © 2017-2021 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Swap.h"

#include <TrustWalletCore/TWCoinType.h>
#include "Coin.h"

// BTC
#include "Bitcoin/SigHashType.h"
#include "../proto/Bitcoin.pb.h"
// ETH
#include "Ethereum/Address.h"
#include "Ethereum/ABI/Function.h"
#include "Ethereum/ABI/ParamBase.h"
#include "Ethereum/ABI/ParamAddress.h"
#include "uint256.h"
#include "../proto/Ethereum.pb.h"
// BNB
#include "Binance/Address.h"
#include "../proto/Binance.pb.h"

#include <cstdlib>

/*
 * References:
 *  https://gitlab.com/thorchain/asgardex-common/asgardex-util
 */

namespace TW::THORChainSwap {

static Data ethAddressStringToData(const std::string& asString) {
    Data asData(20);
    if (asString.empty() || !Ethereum::Address::isValid(asString)) {
        return asData;
    }
    auto address = Ethereum::Address(asString);
    std::copy(address.bytes.begin(), address.bytes.end(), asData.data());
    return asData;
}

TWCoinType chainCoinType(Chain chain) {
    switch (chain) {
        case Chain::ETH: return TWCoinTypeEthereum;
        case Chain::BNB: return TWCoinTypeBinance;
        case Chain::BTC: return TWCoinTypeBitcoin;
        case Chain::THOR:
        default:
            return TWCoinTypeTHORChain;
    }
}

std::string chainName(Chain chain) {
    switch (chain) {
        case Chain::ETH: return "ETH";
        case Chain::BNB: return "BNB";
        case Chain::BTC: return "BTC";
        case Chain::THOR:
        default:
            return "THOR";
    }
}

std::string Swap::buildMemo(Proto::Asset toAsset, const std::string& toAddress, uint64_t limit, const std::string& feeAddress, std::optional<uint16_t> feeRate, const std::string& extra) {
    std::string prefix = "SWAP";
    const auto& toChain = static_cast<Chain>(toAsset.chain());
    const auto& toTokenId = toAsset.token_id();
    const auto& toSymbol = toAsset.symbol();
    if (toChain == Chain::ETH) {
        prefix = "=";
    }
    const auto toCoinToken = (!toTokenId.empty() && toTokenId != "0x0000000000000000000000000000000000000000") ? toTokenId : toSymbol;
    std::stringstream memo;
    memo << prefix + ":" + chainName(toChain) + "." + toCoinToken + ":" + toAddress + ":" + std::to_string(limit);

    if (!feeAddress.empty() || feeRate.has_value() || !extra.empty()) {
        memo << ":";
        if (!feeAddress.empty()) {
            memo << feeAddress;
        }
        if (feeRate.has_value() || !extra.empty()) {
            memo << ":";
            if (feeRate.has_value()) {
                memo << std::to_string(feeRate.value());
            }
            if (!extra.empty()) {
                memo << ":";
                memo << extra;
            }
        }
    }

    return memo.str();
}

bool validateAddress(Chain chain, const std::string& address) {
    return TW::validateAddress(chainCoinType(chain), address);
}

std::tuple<Data, int, std::string> SwapBuilder::build() {
    auto fromChain = static_cast<Chain>(mFromAsset.chain());
    auto toChain = static_cast<Chain>(mToAsset.chain());

    if (!validateAddress(fromChain, mFromAddress)) {
        return std::make_tuple<Data, int, std::string>({}, static_cast<int>(Proto::ErrorCode::Error_Invalid_from_address), "Invalid from address");
    }
    if (!validateAddress(toChain, mToAddress)) {
        return std::make_tuple<Data, int, std::string>({}, static_cast<int>(Proto::ErrorCode::Error_Invalid_to_address), "Invalid to address");
    }

    uint64_t fromAmountNum = std::stoull(mFromAmount);
    const auto memo = this->buildMemo();

    switch (fromChain) {
    case Chain::BTC: {
        return buildBitcoin(fromAmountNum, memo);
    case Chain::BNB:
        return buildBinance(fromAmountNum, memo);
    case Chain::ETH:
        return buildEth(fromAmountNum, memo);
    }
    default:
        return std::make_tuple<Data, int, std::string>({}, static_cast<int>(Proto::ErrorCode::Error_Unsupported_from_chain), "Unsupported from chain: " + std::to_string(fromChain));
    }
}
std::string SwapBuilder::buildMemo() noexcept {
    uint64_t toAmountLimitNum = std::stoull(mToAmountLimit);
    std::optional<uint16_t> feeRateNum = !mAffFeeRate ? std::nullopt : std::make_optional(std::stoull(*mAffFeeRate));

    std::string prefix = "SWAP";
    const auto& toChain = static_cast<Chain>(mToAsset.chain());
    const auto& toTokenId = mToAsset.token_id();
    const auto& toSymbol = mToAsset.symbol();
    if (toChain == Chain::ETH) {
        prefix = "=";
    }
    const auto toCoinToken = (!toTokenId.empty() && toTokenId != "0x0000000000000000000000000000000000000000") ? toTokenId : toSymbol;
    std::stringstream memo;
    memo << prefix + ":" + chainName(toChain) + "." + toCoinToken + ":" + mToAddress + ":" + std::to_string(toAmountLimitNum);

    if (mAffFeeAddress.has_value() || mAffFeeRate.has_value() || mExtraMemo.has_value()) {
        if (mAffFeeAddress.has_value()) {
            memo << ":" << mAffFeeAddress.value();
        }
        if (mAffFeeRate.has_value() || !mExtraMemo.has_value()) {
            memo << ":";
            if (feeRateNum.has_value()) {
                memo << std::to_string(feeRateNum.value());
            }
            if (mExtraMemo.has_value()) {
                memo << ":" << mExtraMemo.value();
            }
        }
    }

    return memo.str();
}

std::tuple<Data, int, std::string> SwapBuilder::buildBitcoin(uint64_t amount, const std::string& memo) {
    auto input = Bitcoin::Proto::SigningInput();
    Data out;
    // Following fields must be set afterwards, before signing ...
    input.set_hash_type(TWBitcoinSigHashTypeAll);
    input.set_byte_fee(1);
    input.set_use_max_amount(false);
    // private_key[]
    // utxo[]
    // scripts[]
    // ... end

    input.set_amount(static_cast<int64_t>(amount));
    input.set_to_address(mVaultAddress);
    input.set_change_address(mFromAddress);
    input.set_coin_type(TWCoinTypeBitcoin);
    input.set_output_op_return(memo);

    auto serialized = input.SerializeAsString();
    out.insert(out.end(), serialized.begin(), serialized.end());
    return std::make_tuple(std::move(out), 0, "");
}
std::tuple<Data, int, std::string> SwapBuilder::buildBinance(uint64_t amount, const std::string& memo) {
    auto input = Binance::Proto::SigningInput();
    Data out;

    // Following fields must be set afterwards, before signing ...
    input.set_chain_id("");
    input.set_account_number(0);
    input.set_sequence(0);
    input.set_source(0);
    input.set_private_key("");
    // ... end

    input.set_memo(memo);

    auto& order = *input.mutable_send_order();

    auto token = Binance::Proto::SendOrder::Token();
    token.set_denom("BNB");
    token.set_amount(amount);
    {
        Binance::Address fromAddressBin;
        Binance::Address::decode(mFromAddress, fromAddressBin);
        auto input_ = order.add_inputs();
        input_->set_address(fromAddressBin.getKeyHash().data(), fromAddressBin.getKeyHash().size());
        *input_->add_coins() = token;
    }
    {
        Binance::Address vaultAddressBin;
        Binance::Address::decode(mVaultAddress, vaultAddressBin);
        auto output = order.add_outputs();
        output->set_address(vaultAddressBin.getKeyHash().data(), vaultAddressBin.getKeyHash().size());
        *output->add_coins() = token;
    }

    auto serialized = input.SerializeAsString();
    out.insert(out.end(), serialized.begin(), serialized.end());
    return std::make_tuple(std::move(out), 0, "");
}

std::tuple<Data, int, std::string> SwapBuilder::buildEth(uint64_t amount, const std::string& memo) {
    Data out;
    auto input = Ethereum::Proto::SigningInput();
    const auto& toTokenId = mToAsset.token_id();
    // some sanity check / address conversion
    Data vaultAddressBin = ethAddressStringToData(mVaultAddress);
    if (!Ethereum::Address::isValid(mVaultAddress) || vaultAddressBin.size() != Ethereum::Address::size) {
        return std::make_tuple(Data(), static_cast<int>(Proto::ErrorCode::Error_Invalid_vault_address), "Invalid vault address: " + mVaultAddress);
    }
    if (!Ethereum::Address::isValid(*mRouterAddress)) {
        return std::make_tuple(Data(), static_cast<int>(Proto::ErrorCode::Error_Invalid_router_address), "Invalid router address: " + *mRouterAddress);
    }
    Data toAssetAddressBin = ethAddressStringToData(toTokenId);

    // Following fields must be set afterwards, before signing ...
    const auto chainId = store(uint256_t(0));
    input.set_chain_id(chainId.data(), chainId.size());
    const auto nonce = store(uint256_t(0));
    input.set_nonce(nonce.data(), nonce.size());
    const auto gasPrice = store(uint256_t(0));
    input.set_gas_price(gasPrice.data(), gasPrice.size());
    const auto gasLimit = store(uint256_t(0));
    input.set_gas_limit(gasLimit.data(), gasLimit.size());
    input.set_private_key("");
    // ... end

    input.set_to_address(*mRouterAddress);
    auto& transfer = *input.mutable_transaction()->mutable_contract_generic();
    auto func = Ethereum::ABI::Function("deposit", std::vector<std::shared_ptr<Ethereum::ABI::ParamBase>>{
                                                       std::make_shared<Ethereum::ABI::ParamAddress>(vaultAddressBin),
                                                       std::make_shared<Ethereum::ABI::ParamAddress>(toAssetAddressBin),
                                                       std::make_shared<Ethereum::ABI::ParamUInt256>(uint256_t(amount)),
                                                       std::make_shared<Ethereum::ABI::ParamString>(memo)
                                                   });
    Data payload;
    func.encode(payload);
    transfer.set_data(payload.data(), payload.size());
    Data amountData = store(uint256_t(amount));
    transfer.set_amount(amountData.data(), amountData.size());

    auto serialized = input.SerializeAsString();
    out.insert(out.end(), serialized.begin(), serialized.end());
    return std::make_tuple(std::move(out), 0, "");
}
} // namespace TW
