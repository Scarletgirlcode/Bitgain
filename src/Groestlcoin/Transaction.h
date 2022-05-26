// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "../Bitcoin/Transaction.h"

namespace TW::Groestlcoin {

struct Transaction : public Bitcoin::Transaction {
    Transaction() : Bitcoin::Transaction(1, 0, Hash::Hash_sha256) {}
    Transaction(int32_t version, uint32_t lockTime = 0) :
        Bitcoin::Transaction(version, lockTime, Hash::Hash_sha256) {}
};

} // namespace TW::Groestlcoin
