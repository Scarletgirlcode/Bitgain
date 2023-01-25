// Copyright © 2017-2021 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Transaction.h"
#include "Ethereum/ABI.h"
#include <Ethereum/EIP4337.h>
#include "HexCoding.h"
#include "RLP.h"

namespace TW::Ethereum {

static const Data EmptyListEncoded = parse_hex("c0");

/// TransactionNonTyped
std::shared_ptr<TransactionNonTyped>
TransactionNonTyped::buildNativeTransfer(const uint256_t& nonce,
                                         const uint256_t& gasPrice, const uint256_t& gasLimit,
                                         const Data& toAddress, const uint256_t& amount, const Data& data) {
    return std::make_shared<TransactionNonTyped>(nonce, gasPrice, gasLimit, toAddress, amount, data);
}

std::shared_ptr<TransactionNonTyped>
TransactionNonTyped::buildERC20Transfer(const uint256_t& nonce,
                                        const uint256_t& gasPrice, const uint256_t& gasLimit,
                                        const Data& tokenContract, const Data& toAddress, const uint256_t& amount) {
    return std::make_shared<TransactionNonTyped>(nonce, gasPrice, gasLimit, tokenContract, 0, buildERC20TransferCall(toAddress, amount));
}

std::shared_ptr<TransactionNonTyped>
TransactionNonTyped::buildERC20Approve(const uint256_t& nonce,
                                       const uint256_t& gasPrice, const uint256_t& gasLimit,
                                       const Data& tokenContract, const Data& spenderAddress, const uint256_t& amount) {
    return std::make_shared<TransactionNonTyped>(nonce, gasPrice, gasLimit, tokenContract, 0, buildERC20ApproveCall(spenderAddress, amount));
}

std::shared_ptr<TransactionNonTyped>
TransactionNonTyped::buildERC721Transfer(const uint256_t& nonce,
                                         const uint256_t& gasPrice, const uint256_t& gasLimit,
                                         const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId) {
    return std::make_shared<TransactionNonTyped>(nonce, gasPrice, gasLimit, tokenContract, 0, buildERC721TransferFromCall(from, to, tokenId));
}

std::shared_ptr<TransactionNonTyped>
TransactionNonTyped::buildERC1155Transfer(const uint256_t& nonce,
                                          const uint256_t& gasPrice, const uint256_t& gasLimit,
                                          const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId, const uint256_t& value, const Data& data) {
    return std::make_shared<TransactionNonTyped>(nonce, gasPrice, gasLimit, tokenContract, 0, buildERC1155TransferFromCall(from, to, tokenId, value, data));
}

Data TransactionNonTyped::preHash(const uint256_t chainID) const {
    return Hash::keccak256(serialize(chainID));
}

Data TransactionNonTyped::serialize(const uint256_t chainID) const {
    Data encoded;
    append(encoded, RLP::encode(nonce));
    append(encoded, RLP::encode(gasPrice));
    append(encoded, RLP::encode(gasLimit));
    append(encoded, RLP::encode(to));
    append(encoded, RLP::encode(amount));
    append(encoded, RLP::encode(payload));
    append(encoded, RLP::encode(chainID));
    append(encoded, RLP::encode(0));
    append(encoded, RLP::encode(0));
    return RLP::encodeList(encoded);
}

Data TransactionNonTyped::encoded(const Signature& signature, [[maybe_unused]] const uint256_t chainID) const {
    Data encoded;
    append(encoded, RLP::encode(nonce));
    append(encoded, RLP::encode(gasPrice));
    append(encoded, RLP::encode(gasLimit));
    append(encoded, RLP::encode(to));
    append(encoded, RLP::encode(amount));
    append(encoded, RLP::encode(payload));
    append(encoded, RLP::encode(signature.v));
    append(encoded, RLP::encode(signature.r));
    append(encoded, RLP::encode(signature.s));
    return RLP::encodeList(encoded);
}

Data TransactionNonTyped::buildERC20TransferCall(const Data& to, const uint256_t& amount) {
    // clang-format off
    auto func = ABI::Function("transfer", std::vector<std::shared_ptr<ABI::ParamBase>>{
        std::make_shared<ABI::ParamAddress>(to),
        std::make_shared<ABI::ParamUInt256>(amount)
    });
    // clang-format on
    Data payload;
    func.encode(payload);
    return payload;
}

Data TransactionNonTyped::buildERC20ApproveCall(const Data& spender, const uint256_t& amount) {
    // clang-format off
    auto func = ABI::Function("approve", std::vector<std::shared_ptr<ABI::ParamBase>>{
        std::make_shared<ABI::ParamAddress>(spender),
        std::make_shared<ABI::ParamUInt256>(amount)
    });
    // clang-format on
    Data payload;
    func.encode(payload);
    return payload;
}

Data TransactionNonTyped::buildERC721TransferFromCall(const Data& from, const Data& to, const uint256_t& tokenId) {
    // clang-format off
    auto func = ABI::Function("transferFrom", std::vector<std::shared_ptr<ABI::ParamBase>>{
        std::make_shared<ABI::ParamAddress>(from),
        std::make_shared<ABI::ParamAddress>(to),
        std::make_shared<ABI::ParamUInt256>(tokenId)
    });
    // clang-format on
    Data payload;
    func.encode(payload);
    return payload;
}

Data TransactionNonTyped::buildERC1155TransferFromCall(const Data& from, const Data& to, const uint256_t& tokenId, const uint256_t& value, const Data& data) {
    // clang-format off
    auto func = ABI::Function("safeTransferFrom", std::vector<std::shared_ptr<ABI::ParamBase>>{
        std::make_shared<ABI::ParamAddress>(from),
        std::make_shared<ABI::ParamAddress>(to),
        std::make_shared<ABI::ParamUInt256>(tokenId),
        std::make_shared<ABI::ParamUInt256>(value),
        std::make_shared<ABI::ParamByteArray>(data)
    });
    // clang-format on
    Data payload;
    func.encode(payload);
    return payload;
}

/// TransactionEip1559
Data TransactionEip1559::preHash(const uint256_t chainID) const {
    return Hash::keccak256(serialize(chainID));
}

Data TransactionEip1559::serialize(const uint256_t chainID) const {
    Data encoded;
    append(encoded, RLP::encode(chainID));
    append(encoded, RLP::encode(nonce));
    append(encoded, RLP::encode(maxInclusionFeePerGas));
    append(encoded, RLP::encode(maxFeePerGas));
    append(encoded, RLP::encode(gasLimit));
    append(encoded, RLP::encode(to));
    append(encoded, RLP::encode(amount));
    append(encoded, RLP::encode(payload));
    append(encoded, EmptyListEncoded); // empty accessList

    Data envelope;
    append(envelope, static_cast<uint8_t>(type));
    append(envelope, RLP::encodeList(encoded));
    return envelope;
}

Data TransactionEip1559::encoded(const Signature& signature, const uint256_t chainID) const {
    Data encoded;
    append(encoded, RLP::encode(chainID));
    append(encoded, RLP::encode(nonce));
    append(encoded, RLP::encode(maxInclusionFeePerGas));
    append(encoded, RLP::encode(maxFeePerGas));
    append(encoded, RLP::encode(gasLimit));
    append(encoded, RLP::encode(to));
    append(encoded, RLP::encode(amount));
    append(encoded, RLP::encode(payload));
    append(encoded, EmptyListEncoded); // empty accessList
    append(encoded, RLP::encode(signature.v));
    append(encoded, RLP::encode(signature.r));
    append(encoded, RLP::encode(signature.s));

    Data envelope;
    append(envelope, static_cast<uint8_t>(type));
    append(envelope, RLP::encodeList(encoded));
    return envelope;
}

std::shared_ptr<TransactionEip1559>
TransactionEip1559::buildNativeTransfer(const uint256_t& nonce,
                                        const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
                                        const Data& toAddress, const uint256_t& amount, const Data& data) {
    return std::make_shared<TransactionEip1559>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, toAddress, amount, data);
}

std::shared_ptr<TransactionEip1559>
TransactionEip1559::buildERC20Transfer(const uint256_t& nonce,
                                       const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
                                       const Data& tokenContract, const Data& toAddress, const uint256_t& amount) {
    return std::make_shared<TransactionEip1559>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC20TransferCall(toAddress, amount));
}

std::shared_ptr<TransactionEip1559>
TransactionEip1559::buildERC20Approve(const uint256_t& nonce,
                                      const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
                                      const Data& tokenContract, const Data& spenderAddress, const uint256_t& amount) {
    return std::make_shared<TransactionEip1559>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC20ApproveCall(spenderAddress, amount));
}

std::shared_ptr<TransactionEip1559>
TransactionEip1559::buildERC721Transfer(const uint256_t& nonce,
                                        const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
                                        const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId) {
    return std::make_shared<TransactionEip1559>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC721TransferFromCall(from, to, tokenId));
}

std::shared_ptr<TransactionEip1559>
TransactionEip1559::buildERC1155Transfer(const uint256_t& nonce,
                                         const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
                                         const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId, const uint256_t& value, const Data& data) {
    return std::make_shared<TransactionEip1559>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC1155TransferFromCall(from, to, tokenId, value, data));
}

/// TransactionEip4337
Data TransactionEip4337::preHash(const uint256_t chainID) const {
    return Hash::keccak256(serialize(chainID));
}

Data TransactionEip4337::serialize(const uint256_t chainID) const {
    auto p = ABI::ParamTuple(std::vector<std::shared_ptr<ABI::ParamBase>>{
        std::make_shared<ABI::ParamAddress>(sender),
        std::make_shared<ABI::ParamUInt256>(nonce),
        std::make_shared<ABI::ParamByteArray>(initCode),
        std::make_shared<ABI::ParamByteArray>(payload),
        std::make_shared<ABI::ParamUInt256>(gasLimit),
        std::make_shared<ABI::ParamUInt256>(verificationGasLimit),
        std::make_shared<ABI::ParamUInt256>(preVerificationGas),
        std::make_shared<ABI::ParamUInt256>(maxFeePerGas),
        std::make_shared<ABI::ParamUInt256>(maxInclusionFeePerGas),
        std::make_shared<ABI::ParamByteArray>(paymasterAndData),
        std::make_shared<ABI::ParamByteArray>(parse_hex("0x"))
    });
    Data encoded;
    p.encode(encoded);

    return Data(encoded.begin(), encoded.end() - 32); // remove trailing word (zero-length signature)
}

Data TransactionEip4337::encoded(const Signature& signature, const uint256_t chainID) const {
    Data encoded;
    append(encoded, RLP::encode(chainID));
    append(encoded, RLP::encode(nonce));
    append(encoded, RLP::encode(maxInclusionFeePerGas));
    append(encoded, RLP::encode(maxFeePerGas));
    append(encoded, RLP::encode(gasLimit));
    append(encoded, RLP::encode(payload));
    append(encoded, EmptyListEncoded); // empty accessList
    append(encoded, RLP::encode(signature.v));
    append(encoded, RLP::encode(signature.r));
    append(encoded, RLP::encode(signature.s));

    Data envelope;
    append(envelope, static_cast<uint8_t>(type));
    append(envelope, RLP::encodeList(encoded));
    return envelope;
}

std::shared_ptr<TransactionEip4337>
TransactionEip4337::buildNativeTransfer(const Data& factoryAddress, const Data& logicAddress, const Data& ownerAddress,
                                        const Data& toAddress, const uint256_t& amount, const uint256_t& nonce,
                                        const uint256_t& gasLimit, const uint256_t& verificationGasLimit, const uint256_t& maxFeePerGas, const uint256_t& maxInclusionFeePerGas, const uint256_t& preVerificationGas,
                                        const Data& paymasterAndData) {
    return std::make_shared<TransactionEip4337>(
        parse_hex(Ethereum::getEIP4337DeploymentAddress(hex(factoryAddress), hex(logicAddress), hex(ownerAddress))),
        nonce,
        Ethereum::getEIP4337AccountInitializeBytecode(hex(ownerAddress), hex(factoryAddress)),
        gasLimit,
        verificationGasLimit,
        maxFeePerGas,
        maxInclusionFeePerGas,
        preVerificationGas,
        Ethereum::getEIP4337ExecuteBytecode(toAddress, amount, {}),
        paymasterAndData
        );
}

//std::shared_ptr<TransactionEip4337>
//TransactionEip4337::buildERC20Transfer(const uint256_t& nonce,
//                                       const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
//                                       const Data& tokenContract, const Data& toAddress, const uint256_t& amount) {
//    return std::make_shared<TransactionEip4337>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC20TransferCall(toAddress, amount));
//}
//
//std::shared_ptr<TransactionEip4337>
//TransactionEip4337::buildERC20Approve(const uint256_t& nonce,
//                                      const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
//                                      const Data& tokenContract, const Data& spenderAddress, const uint256_t& amount) {
//    return std::make_shared<TransactionEip4337>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC20ApproveCall(spenderAddress, amount));
//}
//
//std::shared_ptr<TransactionEip4337>
//TransactionEip4337::buildERC721Transfer(const uint256_t& nonce,
//                                        const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
//                                        const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId) {
//    return std::make_shared<TransactionEip4337>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC721TransferFromCall(from, to, tokenId));
//}
//
//std::shared_ptr<TransactionEip4337>
//TransactionEip4337::buildERC1155Transfer(const uint256_t& nonce,
//                                         const uint256_t& maxInclusionFeePerGas, const uint256_t& maxFeePerGas, const uint256_t& gasPrice,
//                                         const Data& tokenContract, const Data& from, const Data& to, const uint256_t& tokenId, const uint256_t& value, const Data& data) {
//    return std::make_shared<TransactionEip4337>(nonce, maxInclusionFeePerGas, maxFeePerGas, gasPrice, tokenContract, 0, TransactionNonTyped::buildERC1155TransferFromCall(from, to, tokenId, value, data));
//}

} // namespace TW::Ethereum
