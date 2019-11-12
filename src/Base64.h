// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "Data.h"

namespace TW::Base64 {

// Decode a Base64-format string
Data decode(const std::string& val);

// Decode a Base64Url-format or a Base64 string.
// Base64Url format uses '-' and '_' as the two special characters, Base64 uses '+'and '/'.
Data decodeBase64Url(const std::string& val);

// Encode bytes into Base64 string
std::string encode(const Data& val);

} // namespace TW::Base64
