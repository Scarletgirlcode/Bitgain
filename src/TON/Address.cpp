// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Address.h"
#include "Cell.h"
#include "../Base64.h"
#include "../Crc.h"
#include "../HexCoding.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

using namespace TW;
using namespace TW::TON;

bool Workchain::isValid(WorkchainId_t workchainId) {
    return (workchainId == MasterChainId || workchainId == BasicChainId);
}

Address::Address(const std::string& address) {
    bool valid = parseAddress(address, *this);

    // Ensure address is valid
    if (!valid) {
        throw std::invalid_argument("Invalid address data");
    }
}

Address::Address(const PublicKey& publicKey) {
    // Steps: a StateInit account state cell is created (containing code and data), its hash is taken,
    // and new address is derived from the hash

    if (publicKey.type != TWPublicKeyTypeED25519) {
        throw std::invalid_argument("Invalid public key type");
    }

    // code
    // const std::string
    // accountSCCodeFixed("FF0020DDA4F260810200D71820D70B1FED44D0D31FD3FFD15112BAF2A122F901541044F910F2A2F80001D31F3120D74A96D307D402FB00DED1A4C8CB1FCBFFC9ED54");
    const std::string accountSCCodeFixed(
        "FF0020DDA4F260810200D71820D70B1FED44D0D7091FD709FFD15112BAF2A122F901541044F910F2A2F80001D7"
        "091F3120D74A97D70907D402FB00DED1A4C8CB1FCBFFC9ED54");
    auto ccode = std::make_shared<Cell>();
    ccode->setSliceBytesStr(accountSCCodeFixed);

    // data: 4 byte serial num (0), 32 byte public key
    Data data;
    append(data, Data(4));
    append(data, publicKey.bytes);
    assert(data.size() == 4 + 32);
    auto cdata = std::make_shared<Cell>();
    cdata->setSliceBytes(data);

    Cell stateInit;
    stateInit.setSliceBitsStr("34", 5);
    stateInit.addCell(ccode);
    stateInit.addCell(cdata);

    // compute hash
    auto hash = stateInit.hash();

    // fill members
    workchainId = Workchain::MasterChainId;
    addrBytes = hash;
    isBounceable = true;
    isTestOnly = false;
}

bool Address::isValid(const std::string& address) {
    Address addr;
    bool isValid = parseAddress(address, addr);
    return isValid;
}

bool Address::parseAddress(const std::string& addressStr_in, Address& addr_inout) {
    // try several formats, start with the common one, stop if one matches
    bool isValidUser = parseUserAddress(addressStr_in, addr_inout);
    if (isValidUser) {
        return true;
    }
    bool isValidRaw = parseRawAddress(addressStr_in, addr_inout);
    return isValidRaw;
}

bool Address::parseRawAddress(const std::string& addressStr_in, Address& addr_inout) {
    // split by colon ':'
    auto colidx = addressStr_in.find(':');

    if (colidx == std::string::npos) {
        // no colon, invalid
        return false;
    }
    if (colidx < 1 || colidx >= addressStr_in.length() - 1) {
        // colon in wrong position
        return false;
    }
    std::string workchainStr = addressStr_in.substr(0, colidx);
    std::string addressStr = addressStr_in.substr(colidx + 1, addressStr_in.length() - colidx - 1);

    WorkchainId_t workchainId;
    try {
        workchainId = std::stoi(workchainStr);
    } catch (const std::exception& e) {
        // workchain ID is invalid (not a decimal number)
        return false;
    }
    if (!Workchain::isValid(workchainId)) {
        // invalid workchain ID
        return false;
    }
    addr_inout.workchainId = workchainId;

    if (addressStr.length() != AddressLength * 2) {
        // wrong length of address part
        return false;
    }

    addr_inout.addrBytes = parse_hex(addressStr);

    addr_inout.isBounceable = true;
    addr_inout.isTestOnly = false;

    return true;
}

bool Address::parseUserAddress(const std::string& addressStr_in, Address& addr_inout) {
    Data bytes;
    try {
        bytes = Base64::decodeBase64Url(addressStr_in);
    } catch (const std::exception& ex) {
        return false;
    }

    // check length
    if (bytes.size() != 36) {
        return false;
    }
    assert(bytes.size() >= 36);

    addr_inout.isBounceable = true;
    addr_inout.isTestOnly = false;
    byte tagByte = bytes[0];
    if (tagByte >= 0x80) {
        // test-only
        tagByte -= 0x80;
        addr_inout.isTestOnly = true;
    }
    if (tagByte >= 0x40) {
        // not bounceable
        tagByte -= 0x40;
        addr_inout.isBounceable = false;
    }
    if (tagByte != 0x11) {
        // invalid tag
        return false;
    }

    byte chainId = bytes[1];
    switch (chainId) {
    case 0x00:
        addr_inout.workchainId = Workchain::BasicChainId;
        break;
    case 0xff:
        addr_inout.workchainId = Workchain::MasterChainId;
        break;
    default:
        return false; // invalid chain
    }

    // 32 bytes address
    addr_inout.addrBytes = Data(AddressLength);
    std::copy(bytes.begin() + 2, bytes.begin() + 2 + AddressLength, addr_inout.addrBytes.begin());

    // check CRC
    uint16_t crcGiven = static_cast<uint16_t>(bytes[34] << 8) + bytes[35];
    uint16_t crcComputed = Crc::crc16(bytes.data(), 2 + AddressLength);
    if (crcGiven != crcComputed) {
        // CRC mismatch
        return false;
    }

    return true;
}

std::string Address::string() const {
    Data bytes;
    // tag
    byte tag = 0x11 + 0x40 * (isBounceable ? 0 : 1) + 0x80 * (isTestOnly ? 1 : 0);
    bytes.push_back(tag);
    byte chainId = 0;
    switch (workchainId) {
    case Workchain::BasicChainId:
        chainId = 0x00;
        break;
    case Workchain::MasterChainId:
        chainId = 0xff;
        break;
    default:
        chainId = 0x01;
        break; // invalid
    }
    bytes.push_back(chainId);
    append(bytes, addrBytes);
    // add crc checksumS
    uint16_t crc = Crc::crc16(bytes.data(), static_cast<uint32_t>(bytes.size()));
    bytes.push_back(crc >> 8);
    bytes.push_back(crc & 0xff);

    // base64 encode
    std::string addrEnc = Base64::encode(bytes);
    return addrEnc;
}

std::string Address::stringRaw() const {
    std::stringstream ss;
    ss << workchainId << ':' << hex(addrBytes);
    return ss.str();
}
