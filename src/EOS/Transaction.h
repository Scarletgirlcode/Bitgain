#pragma once

#include <nlohmann/json.hpp>

#include "../Bravo/Serialization.h"
#include "Action.h"
#include "../Data.h"
#include "../PrivateKey.h"
#include "Prefixes.h"

#include <set>
#include <array>

namespace TW::EOS {

class Signature: Bravo::Serializable
{
public:
    Data data;
    Type type;

    static const size_t DataSize = 65;
    static const size_t ChecksumSize = 4;

    Signature(Data sig, Type type);
    virtual ~Signature() { }
    void serialize(Data& os) const noexcept;
    std::string string() const noexcept;
};

class Extension: Bravo::Serializable {
public:
    uint16_t type;
    Data buffer;

    Extension(uint16_t type, Data buffer) : type(type), buffer(buffer) { }
    virtual ~Extension() { }
    void serialize(Data& os) const noexcept;
    nlohmann::json serialize() const noexcept;
};

class Transaction: Bravo::Serializable
{
public:
    Transaction(const Data& referenceBlockId, int32_t referenceBlockTime);

    void serialize(Data& os) const noexcept;
    nlohmann::json serialize() const noexcept;

    inline bool isValid() { return maxNetUsageWords < UINT32_MAX / 8UL; }

    uint16_t refBlockNumber = 0;
    uint32_t refBlockPrefix = 0;
    int32_t expiration = 0;
    uint32_t maxNetUsageWords = 0;
    uint8_t maxCPUUsageInMS = 0;
    uint32_t delaySeconds = 0;

    std::vector<Action> actions;
    std::vector<Action> contextFreeActions;
    std::vector<Extension> transactionExtensions;
    std::vector<Signature> signatures;

    Data contextFreeData;

    void setReferenceBlock(const Data& referenceBlockId);

    static const uint32_t ExpirySeconds = 30;
    // static const uint32_t ExpirySeconds = 60 * 10;
};
} // namespace TW::EOS