// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Signer.h"

#include "Base64.h"

#include "TheOpenNetwork/wallet/WalletV4R2.h"
#include "Everscale/WorkchainType.h"

namespace TW::TheOpenNetwork {

Data Signer::createTransferMessage(const PublicKey& publicKey, const PrivateKey& privateKey, const Proto::Transfer& transfer) {
    std::unique_ptr<Wallet> wallet;
    const int8_t workchainId = Everscale::WorkchainType::Basechain;

    switch (transfer.wallet_version()) {
    case Proto::WalletVersion::WalletV4R2: {
        wallet.reset(new WalletV4R2(publicKey, workchainId));
        break;
    }
    default:
        throw std::invalid_argument("Unsupported wallet version");
    }

    const auto msg = wallet->createTransferMessage(
        privateKey,
        Address(transfer.dest()),
        transfer.amount(),
        transfer.seqno(),
        transfer.mode(),
        transfer.expired_at(),
        transfer.comment()
    );

    Data result{};
    msg->serialize(result);
    return result;
}

Proto::SigningOutput Signer::sign(const Proto::SigningInput &input) noexcept {
    const auto& privateKey = PrivateKey(input.private_key());
    const auto& publicKey = privateKey.getPublicKey(TWPublicKeyTypeED25519);

    auto protoOutput = Proto::SigningOutput();

    switch (input.action_oneof_case()) {
    case Proto::SigningInput::ActionOneofCase::kTransfer: {
        const auto& transfer = input.transfer();

        try {
            const auto& transferMessage = Signer::createTransferMessage(publicKey, privateKey, transfer);
            protoOutput.set_encoded(TW::Base64::encode(transferMessage));
        } catch (...) { }
        break;
    }
    default:
        break;
    }
    return protoOutput;
}

} // namespace TW::TheOpenNetwork
