// Copyright © 2017-2021 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "JsonSerialization.h"

#include "Base64.h"
#include "PrivateKey.h"

using namespace TW;
using namespace TW::Cosmos;

using json = nlohmann::json;
using string = std::string;

namespace TW::Cosmos {

const string TYPE_PREFIX_MSG_SEND = "cosmos-sdk/MsgSend";
const string TYPE_PREFIX_MSG_DELEGATE = "cosmos-sdk/MsgDelegate";
const string TYPE_PREFIX_MSG_UNDELEGATE = "cosmos-sdk/MsgUndelegate";
const string TYPE_PREFIX_MSG_REDELEGATE = "cosmos-sdk/MsgBeginRedelegate";
const string TYPE_PREFIX_MSG_WITHDRAW_REWARD = "cosmos-sdk/MsgWithdrawDelegationReward";
const string TYPE_PREFIX_PUBLIC_KEY = "tendermint/PubKeySecp256k1";

const string TYPE_PREFIX_MSG_EXECUTE_CONTRACT = "wasm/MsgExecuteContract"; //from terra

static string broadcastMode(Proto::BroadcastMode mode) {
    switch (mode) {
    case Proto::BroadcastMode::BLOCK:
        return "block";
    case Proto::BroadcastMode::ASYNC:
        return "async";
    default: return "sync";
    }
}

static json broadcastJSON(json& j, Proto::BroadcastMode mode) {
    return {
        {"tx", j},
        {"mode", broadcastMode(mode)}
    };
}

static json amountJSON(const Proto::Amount& amount) {
    return {
        {"amount", amount.amount()},
        {"denom", amount.denom()}
    };
}

static json amountsJSON(const ::google::protobuf::RepeatedPtrField<Proto::Amount>& amounts) {
    json j = json::array();
    for (auto& amount : amounts) {
        j.push_back(amountJSON(amount));
    }
    return j;
}

static json feeJSON(const Proto::Fee& fee) {
    json js = json::array();

    for (auto& amount : fee.amounts()) {
        js.push_back(amountJSON(amount));
    }

    return {
        {"amount", js},
        {"gas", std::to_string(fee.gas())}
    };
}

static json messageSend(const Proto::Message_Send& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_SEND : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"amount", amountsJSON(message.amounts())},
            {"from_address", message.from_address()},
            {"to_address", message.to_address()}
        }}
    };
}

static json messageDelegate(const Proto::Message_Delegate& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_DELEGATE : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"amount", amountJSON(message.amount())},
            {"delegator_address", message.delegator_address()},
            {"validator_address", message.validator_address()}
        }}
    };
}

static json messageUndelegate(const Proto::Message_Undelegate& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_UNDELEGATE : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"amount", amountJSON(message.amount())},
            {"delegator_address", message.delegator_address()},
            {"validator_address", message.validator_address()}
        }}
    };
}

static json messageRedelegate(const Proto::Message_BeginRedelegate& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_REDELEGATE : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"amount", amountJSON(message.amount())},
            {"delegator_address", message.delegator_address()},
            {"validator_src_address", message.validator_src_address()},
            {"validator_dst_address", message.validator_dst_address()},
        }}
    };
}

static json messageWithdrawReward(const Proto::Message_WithdrawDelegationReward& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_WITHDRAW_REWARD : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"delegator_address", message.delegator_address()},
            {"validator_address", message.validator_address()}
        }}
    };
}

// https://docs.terra.money/Tutorials/Smart-contracts/Manage-CW20-tokens.html#interacting-with-cw20-contract
static json messageExecuteContract(const Proto::Message_ExecuteContract& message) {
    auto typePrefix = message.type_prefix().empty() ? TYPE_PREFIX_MSG_EXECUTE_CONTRACT : message.type_prefix();

    return {
        {"type", typePrefix},
        {"value", {
            {"sender", message.sender()},
            {"contract", message.contract()},
            {"execute_msg", message.execute_msg()},
            {"coins", amountsJSON(message.coins())}
        }}
    };
}

static json messageRawJSON(const Proto::Message_RawJSON& message) {
    return {
        {"type", message.type()},
        {"value", json::parse(message.value())},
    };
}
static json messagesJSON(const Proto::SigningInput& input) {
    json j = json::array();
    for (auto& msg : input.messages()) {
        if (msg.has_send_coins_message()) {
            j.push_back(messageSend(msg.send_coins_message()));
        } else if (msg.has_stake_message()) {
            j.push_back(messageDelegate(msg.stake_message()));
        } else if (msg.has_unstake_message()) {
            j.push_back(messageUndelegate(msg.unstake_message()));
        } else if (msg.has_withdraw_stake_reward_message()) {
            j.push_back(messageWithdrawReward(msg.withdraw_stake_reward_message()));
        } else if (msg.has_restake_message()) {
            j.push_back(messageRedelegate(msg.restake_message()));
        } else if (msg.has_raw_json_message()) {
            j.push_back(messageRawJSON(msg.raw_json_message()));
        } else if (msg.has_execute_contract_message()) {
            j.push_back(messageExecuteContract(msg.execute_contract_message()));
        }
    }
    return j;
}

static json signatureJSON(const Data& signature, const Data& pubkey) {
    return {
        {"pub_key", {
            {"type", TYPE_PREFIX_PUBLIC_KEY},
            {"value", Base64::encode(pubkey)}
        }},
        {"signature", Base64::encode(signature)}
    };
}

json signaturePreimageJSON(const Proto::SigningInput& input) {
    return {
        {"account_number", std::to_string(input.account_number())},
        {"chain_id", input.chain_id()},
        {"fee", feeJSON(input.fee())},
        {"memo", input.memo()},
        {"msgs", messagesJSON(input)},
        {"sequence", std::to_string(input.sequence())}
    };
}

json transactionJSON(const Proto::SigningInput& input, const PublicKey& publicKey, const Data& signature) {
    json tx = {
        {"fee", feeJSON(input.fee())},
        {"memo", input.memo()},
        {"msg", messagesJSON(input)},
        {"signatures", json::array({
            signatureJSON(signature, Data(publicKey.bytes))
        })}
    };
    return broadcastJSON(tx, input.mode());
}

std::string buildJsonTxRaw(const Proto::SigningInput& input, const PublicKey& publicKey, const Data& signature) {
    return transactionJSON(input, publicKey, signature).dump();
}

} // namespace
