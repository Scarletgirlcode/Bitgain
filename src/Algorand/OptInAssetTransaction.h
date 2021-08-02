// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Address.h"
#include "BaseTransaction.h"
#include "../Data.h"
#include "../proto/Algorand.pb.h"

namespace TW::Algorand {

class OptInAssetTransaction: public BaseTransaction {
  public:
    Address address;
    uint64_t fee;
    uint64_t assetId;
    uint64_t firstRound;
    uint64_t lastRound;
    Data note;
    std::string type;

    std::string genesisId;
    Data genesisHash;

    OptInAssetTransaction(Address &address, uint64_t fee, uint64_t assetId, uint64_t firstRound,
                          uint64_t lastRound, Data& note, std::string type, std::string& genesisId, Data& genesisHash)
        : address(address), fee(fee)
        , assetId(assetId), firstRound(firstRound)
        , lastRound(lastRound), note(note)
        , type(type), genesisId(genesisId)
        , genesisHash(genesisHash) {}

  public:
    Data serialize() const override;
    Data serialize(Data& signature) const override;
};

} // namespace TW::Algorand
