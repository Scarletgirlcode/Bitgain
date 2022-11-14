// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "AESParameters.h"
#include "Data.h"
#include "PBKDF2Parameters.h"
#include "ScryptParameters.h"
#include <TrustWalletCore/TWStoredKeyEncryptionLevel.h>

#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace TW::Keystore {

/// Set of parameters used when encoding
struct EncryptionParameters {
    static EncryptionParameters getPreset(enum TWStoredKeyEncryptionLevel preset) {
        switch (preset) {
        case TWStoredKeyEncryptionLevelMinimal:
            return EncryptionParameters(AESParameters(), ScryptParameters::Minimal);
        case TWStoredKeyEncryptionLevelWeak:
        case TWStoredKeyEncryptionLevelDefault:
        default:
            return EncryptionParameters(AESParameters(), ScryptParameters::Weak);
        case TWStoredKeyEncryptionLevelStandard:
            return EncryptionParameters(AESParameters(), ScryptParameters::Standard);
        }
    }

    enum AESSize: std::int32_t {
        Uninitialized = 0,
        A128 = 16,
        A256 = 32,
    };

    std::int32_t getKeyBytesSize() const noexcept {
        if (this->cipher == "aes-128-ctr" || this->cipher == "aes-128-cbc") {
            return A128;
        } else if (this->cipher == "aes-256-ctr") {
            return A256;
        }
        return Uninitialized;
    }

    /// Cipher algorithm.
    std::string cipher{"aes-128-ctr"};

    /// Cipher parameters.
    AESParameters cipherParams = AESParameters();

    /// Key derivation function parameters.
    std::variant<ScryptParameters, PBKDF2Parameters> kdfParams = ScryptParameters();

    EncryptionParameters() = default;

    /// Initializes with standard values.
    EncryptionParameters(AESParameters cipherParams, std::variant<ScryptParameters, PBKDF2Parameters> kdfParams, std::string cipher = "aes-128-ctr")
        : cipher(std::move(cipher)), cipherParams(std::move(cipherParams)), kdfParams(std::move(kdfParams)) {}

    /// Initializes with a JSON object.
    EncryptionParameters(const nlohmann::json& json);

    /// Saves `this` as a JSON object.
    nlohmann::json json() const;

    EncryptionParameters(const EncryptionParameters& other) = default;
    EncryptionParameters(EncryptionParameters&& other) = default;
    EncryptionParameters& operator=(const EncryptionParameters& other) = default;
    EncryptionParameters& operator=(EncryptionParameters&& other) = default;

    virtual ~EncryptionParameters() = default;
};

/// Errors thrown when decrypting a key.
enum class DecryptionError {
    unsupportedKDF,
    unsupportedCipher,
    unsupportedCoin,
    invalidKeyFile,
    invalidCipher,
    invalidPassword,
};

/// An encrypted payload data
struct EncryptedPayload {
public:
    EncryptionParameters params;

    /// Encrypted data.
    Data encrypted;

    /// Message authentication code.
    Data _mac;

    EncryptedPayload() = default;

    /// Initializes with standard values.
    EncryptedPayload(const EncryptionParameters& params, const Data& encrypted, const Data& mac)
        : params(std::move(params))
        , encrypted(std::move(encrypted))
        , _mac(std::move(mac)) {}

    /// Initializes by encrypting data with a password
    /// using standard values.
    EncryptedPayload(const Data& password, const Data& data, const EncryptionParameters& params);

    /// Initializes with a JSON object.
    EncryptedPayload(const nlohmann::json& json);

    /// Decrypts the payload with the given password.
    Data decrypt(const Data& password) const;

    /// Saves `this` as a JSON object.
    nlohmann::json json() const;

    EncryptedPayload(const EncryptedPayload& other) = default;
    EncryptedPayload(EncryptedPayload&& other) = default;
    EncryptedPayload& operator=(const EncryptedPayload& other) = default;
    EncryptedPayload& operator=(EncryptedPayload&& other) = default;

    virtual ~EncryptedPayload();
};

} // namespace TW::Keystore
