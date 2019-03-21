// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include <unordered_map>

#include "ParamsBuilder.h"

using namespace TW;
using namespace TW::Ontology;

void ParamsBuilder::buildNeoVmParam(ParamsBuilder &builder, const boost::any &param) {
    if (param.type() == typeid(std::string)) {
        builder.push(boost::any_cast<std::string>(param));
    } else if (param.type() == typeid(std::array<uint8_t, 20>)) {
        builder.push(boost::any_cast<std::array<uint8_t, 20>>(param));
    } else if (param.type() == typeid(std::vector<uint8_t>)) {
        builder.push(boost::any_cast<std::vector<uint8_t>>(param));
    } else if (param.type() == typeid(uint64_t)) {
        builder.push(boost::any_cast<uint64_t>(param));
    } else if (param.type() == typeid(std::vector<boost::any>)) {
        auto paramVec = boost::any_cast<std::vector<boost::any>>(param);
        for (const auto &item : paramVec) {
            ParamsBuilder::buildNeoVmParam(builder, item);
        }
        builder.push(static_cast<uint8_t>(paramVec.size()));
        builder.pushBack((uint8_t) 0xC1);
    } else if (param.type() == typeid(std::unordered_map<std::string, boost::any>)) {
        builder.pushBack((uint8_t) 0x00);
        builder.pushBack((uint8_t) 0xC6);
        builder.pushBack((uint8_t) 0x6B);
        for (auto const &p : boost::any_cast<std::unordered_map<std::string, boost::any>>(param)) {
            ParamsBuilder::buildNeoVmParam(builder, p.second);
            builder.pushBack((uint8_t) 0x6A);
            builder.pushBack((uint8_t) 0x7C);
            builder.pushBack((uint8_t) 0xC8);
        }
        builder.pushBack((uint8_t) 0x6C);
    } else {
        throw std::runtime_error("Unsupported param type.");
    }
}

void ParamsBuilder::buildNeoVmParam(ParamsBuilder &builder, const std::string &param) {
    builder.pushBack(param);
}

void ParamsBuilder::buildNeoVmParam(ParamsBuilder &builder, const std::array<uint8_t, 20> &param) {
    builder.pushBack(std::vector<uint8_t>(param.begin(), param.end()));
}

void ParamsBuilder::buildNeoVmParam(ParamsBuilder &builder, const std::vector<uint8_t> &param) {
    builder.push(param);
}

void ParamsBuilder::pushVar(const std::vector<uint8_t> &data) {
    pushVar(data.size());
    bytes.insert(bytes.end(), data.begin(), data.end());
}

template<typename T>
void ParamsBuilder::pushVar(T data) {
    if (data < (T) 0xFD) {
        ParamsBuilder::pushBack((uint8_t)
                                        data);
    } else if (data < (T) 0xFFFF) {
        bytes.push_back(0xFD);
        encode16LE(static_cast<uint16_t>(data), bytes);
    } else if (data < (T) 0xFFFFFFFF) {
        bytes.push_back(0xFE);
        encode32LE((uint32_t) data, bytes);
    } else {
        bytes.push_back(0xFF);
        encode64LE(data, bytes);
    }
}

void ParamsBuilder::push(const std::string &data) {
    push(std::vector<uint8_t>(data.begin(), data.end()));
}

void ParamsBuilder::push(const std::array<uint8_t, 20> &data) {
    push(std::vector<uint8_t>(data.begin(), data.end()));
}

void ParamsBuilder::push(const std::vector<uint8_t> &data) {
    auto dataSize = data.size();
    if (dataSize < 75) {
        bytes.push_back(static_cast<uint8_t>(dataSize));
    } else if (dataSize < 256) {
        bytes.push_back(0x4C);
        bytes.push_back(static_cast<uint8_t>(dataSize));
    } else if (dataSize < 65536) {
        bytes.push_back(0x4D);
        encode16LE(static_cast<uint16_t>(dataSize), bytes);
    } else {
        bytes.push_back(0x4E);
        encode32LE(static_cast<uint16_t>(dataSize), bytes);
    }
    bytes.insert(bytes.end(), data.begin(), data.end());
}

void ParamsBuilder::push(uint64_t num) {
    if (num == 0) {
        bytes.push_back(0x00);
    } else if (num < 16) {
        num += 80;
        bytes.push_back(static_cast<uint8_t>(num));
    } else if (num < 128) {
        push(std::vector<uint8_t>{static_cast<uint8_t>(num)});
    } else {
        push(std::vector<uint8_t>{static_cast<uint8_t>(num), static_cast<uint8_t>((num >> 8))});
    }
}

void ParamsBuilder::pushBack(uint8_t data) {
    bytes.push_back(data);
}

void ParamsBuilder::pushBack(uint32_t data) {
    encode32LE(data, bytes);
}

void ParamsBuilder::pushBack(uint64_t data) {
    encode64LE(data, bytes);
}

void ParamsBuilder::pushBack(const std::string &data) {
    bytes.insert(bytes.end(), data.begin(), data.end());
}

void ParamsBuilder::pushBack(const std::array<uint8_t, 20> &data) {
    bytes.insert(bytes.end(), data.begin(), data.end());
}

template<typename T>
void ParamsBuilder::pushBack(const std::vector<T> &data) {
    bytes.insert(bytes.end(), data.begin(), data.end());
}

void ParamsBuilder::push(uint8_t num) {
    if (num == 0) {
        bytes.push_back(0x00);
    } else if (num < 16) {
        num += 80;
        bytes.push_back(static_cast<uint8_t>(num));
    } else if (num < 128) {
        push(std::vector<uint8_t>{num});
    } else {
        push(std::vector<uint8_t>{num, 0x00});
    }
}

std::vector<uint8_t> ParamsBuilder::buildNativeInvokeCode(const std::vector<uint8_t> &contractAddress, uint8_t version, const std::string &method, const boost::any &params) {
    ParamsBuilder builder;
    ParamsBuilder::buildNeoVmParam(builder, params);
    builder.push(std::vector<uint8_t>(method.begin(), method.end()));
    builder.push(contractAddress);
    builder.push(version);
    builder.pushBack((uint8_t) 0x68);
    std::string nativeInvoke = "Ontology.Native.Invoke";
    builder.push(std::vector<uint8_t>(nativeInvoke.begin(), nativeInvoke.end()));
    return builder.getBytes();
}