// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "../Data.h"

namespace TW::Algorand {

class BaseTransaction {
  public:
    virtual Data serialize() const = 0;
    virtual Data serialize(Data& signature) const;
};

} // namespace TW::Algorand