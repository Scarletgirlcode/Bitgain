// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

package com.trustwallet.core.app.blockchains.juno

import com.trustwallet.core.app.utils.toHexByteArray
import org.junit.Assert
import org.junit.Test
import wallet.core.jni.*

class TestJunoAddress {
    init {
        System.loadLibrary("TrustWalletCore")
    }

    @Test
    fun testAnyAddressValidation() {
        val addr = "juno1gckvjxau7k56f8wg8c8xj80khyp83y8x8eqc94"
        val anyAddr = AnyAddress(addr, CoinType.COSMOS, "juno")
        assert(AnyAddress.isValidWithHrp(anyAddr.description(), CoinType.COSMOS, "juno"))
        assert(!AnyAddress.isValidWithHrp(anyAddr.description(), CoinType.BITCOIN, "juno"))
        assert(!AnyAddress.isValid(anyAddr.description(), CoinType.BITCOIN))
        assert(!AnyAddress.isValid(anyAddr.description(), CoinType.COSMOS))
    }

    @Test
    fun testAnyAddressFromPubkey() {
        val pubKey = PublicKey("02753f5c275e1847ba4d2fd3df36ad00af2e165650b35fe3991e9c9c46f68b12bc".toHexByteArray(), PublicKeyType.SECP256K1)
        val anyAddr = AnyAddress(pubKey, CoinType.COSMOS, "juno")
        Assert.assertEquals(anyAddr.description(), "juno1cj2vfjec3c3luf9fx9vddnglhh9gawmncn4k5n");
    }
}
