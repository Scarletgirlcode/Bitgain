// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "CellDep.h"

#include "../BinaryCoding.h"

using namespace TW::Nervos;

void CellDep::encode(Data& data) const {
    outPoint.encode(data);
    data.push_back(depType);
}
