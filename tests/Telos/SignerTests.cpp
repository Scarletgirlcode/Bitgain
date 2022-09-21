// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Telos/Signer.h"
#include "Telos/Address.h"
#include "HexCoding.h"
#include "PrivateKey.h"
#include "PublicKey.h"

#include <gtest/gtest.h>

namespace TW::Telos::tests {

// TODO: Add tests

TEST(TelosSigner, Sign) {
    // TODO: Finalize test implementation

    //auto key = PrivateKey(parse_hex("__PRIVKEY_DATA__"));
    //auto publicKey = key.getPublicKey(TWPublicKeyTypeED25519);
    //auto from = Address(publicKey);
    //auto to = Address("__TO_ADDRESS__");
    //...
    //auto transaction = Transaction(...)
    //auto signature = Signer::sign(key, transaction);
    //auto result = transaction.serialize(signature);

    //ASSERT_EQ(hex(serialized), "__RESULT__");
    //ASSERT_EQ(...)
}

} // namespace TW::Telos::tests
