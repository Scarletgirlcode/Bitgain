#include <TrustWalletCore/TWSteemSigner.h>

#include "../Bravo/Signer.h"
#include "../proto/Bravo.pb.h"
#include "../proto/Common.pb.h"

const std::string MainNetAssetSymbol = "STEEM";
const std::string TestNetAssetSymbol = "TESTS";

using namespace TW::Bravo;

static TW_Proto_Result createErrorResult(const std::string& description) {

    auto result = TW::Proto::Result();
    result.set_success(false);
    result.set_error(description);
    auto serialized = result.SerializeAsString();
    return TWDataCreateWithBytes(reinterpret_cast<const uint8_t *>(serialized.data()), serialized.size());
}

TW_Proto_Result TWSteemSignerSign(TW_Bravo_Proto_SigningInput input) {
    Proto::SigningInput in;
    bool success = in.ParseFromArray(TWDataBytes(input), static_cast<int>(TWDataSize(input)));

    if (!success) {
        return createErrorResult("Error parsing the input.");
    }

    // ensure the amount is within the limits of int64
    if (in.amount() > static_cast<double>(INT64_MAX) / Asset::precision
         || in.amount() < static_cast<double>(INT64_MIN) / Asset::precision) {
        return createErrorResult("Amount out of range!");
    }

    int64_t amount = static_cast<int64_t>(in.amount());

    try {
        // create a transfer operation
		Asset asset{amount, Asset::decimals, in.testnet() ? TestNetAssetSymbol : MainNetAssetSymbol};
        auto op = new TransferOperation(in.sender(), in.recipient(), asset, in.memo());

        // create a Transaction and add the transfer operation
        Transaction tx{ TW::Data(in.reference_block_id().begin(), in.reference_block_id().end()), 
						in.reference_block_time() };
        tx.addOperation(op);

        // sign the transaction with a Signer
        auto key = TW::PrivateKey(TW::Data(in.private_key().begin(), in.private_key().end()));
        auto chainId = TW::Data(in.chain_id().begin(), in.chain_id().end());
        Signer(chainId).sign(key, tx, nullptr);

        // add transaction's json encoding to Signing Output and return that
        Proto::SigningOutput out;
        out.set_json_encoded(tx.serialize().dump());

        auto result = TW::Proto::Result();
        result.set_success(true);
        result.add_objects()->PackFrom(out);
        auto serialized = result.SerializeAsString();
        return TWDataCreateWithBytes(reinterpret_cast<const uint8_t *>(serialized.data()), serialized.size());
    } catch (const std::exception& e) {
        return createErrorResult(e.what());
    }
}