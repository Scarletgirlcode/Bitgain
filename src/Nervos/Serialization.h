// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "../BinaryCoding.h"
#include "../Data.h"
#include "../uint256.h"
#include <numeric>

namespace TW::Nervos {

class Serialization {
public:
    static void encodeDataArray(const std::vector<Data>& dataArray, Data& data) {
        uint32_t dataLength = std::accumulate(dataArray.begin(), dataArray.end(), uint32_t(0),
                                              [](const uint32_t total, const Data& element) {
                                                  return total + uint32_t(element.size());
                                              });
        uint32_t headerLength = 4 + 4 * uint32_t(dataArray.size());
        uint32_t fullLength = headerLength + dataLength;
        encode32LE(fullLength, data);
        uint32_t offset = headerLength;
        for (auto& element : dataArray) {
            encode32LE(offset, data);
            offset += uint32_t(element.size());
        }
        for (auto& element : dataArray) {
            data.insert(data.end(), element.begin(), element.end());
        }
    }

    static Data encodeUint256(uint256_t number, byte minLen = 0) {
        auto data = store(number, minLen);
        std::reverse(data.begin(), data.end());
        return data;
    }

    static uint256_t decodeUint256(const Data& data, byte minLen = 0) {
        auto data1 = Data(data);
        std::reverse(data1.begin(), data1.end());
        return load(data1);
    }
};
} // namespace TW::Nervos
