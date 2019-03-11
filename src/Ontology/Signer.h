// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "PrivateKey.h"

namespace TW {
namespace Ontology {

    class Signer {

    public:
        static PrivateKey getPrivateKey(const std::string &hexPrvKey) noexcept;

        static PublicKey getPublicKey(const PrivateKey &privateKey) noexcept;

        static PublicKey getPublicKey(const std::string &privateKey) noexcept;

        static Data sign(const PrivateKey &privateKey, const Data &msg) noexcept;
    };
}
}
