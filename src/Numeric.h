// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include <numeric>

namespace TW {

template <std::unsigned_integral T>
bool checkAddUnsignedOverflow(T x, T y) {
    return x > std::numeric_limits<T>::max() - y;
}

} // namespace TW
