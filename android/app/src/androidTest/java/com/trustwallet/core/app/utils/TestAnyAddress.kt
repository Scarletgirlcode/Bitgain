// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

package com.trustwallet.core.app.utils

import com.trustwallet.core.app.utils.Numeric
import com.trustwallet.core.app.utils.toHex
import com.trustwallet.core.app.utils.toHexByteArray
import wallet.core.jni.CoinType
import wallet.core.jni.Curve
import wallet.core.jni.Derivation
import wallet.core.jni.HDWallet
import java.security.InvalidParameterException
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Test

class TestAnyAddress {
    init {
        System.loadLibrary("TrustWalletCore");
    }

    val ANY_ADDRESS_TEST_ADDRESS = "bc1qcj2vfjec3c3luf9fx9vddnglhh9gawmncmgxhz"
    val ANY_ADDRESS_TEST_PUBKEY = "02753f5c275e1847ba4d2fd3df36ad00af2e165650b35fe3991e9c9c46f68b12bc"

    @Test
    fun testCreateWithString() {
        val coin = CoinType.BITCOIN
        val address = AnyAddress(ANY_ADDRESS_TEST_ADDRESS, coin)
        assertEquals(address.coin(), coin)
        assertEquals(address.description(), ANY_ADDRESS_TEST_ADDRESS)
    }

    @Test
    fun testCreateWithStringBech32() {
        val coin = CoinType.BITCOIN
        val address1 = AnyAddress(ANY_ADDRESS_TEST_ADDRESS, coin, "bc")
        assertEquals(address1.description(), ANY_ADDRESS_TEST_ADDRESS)

        val address2 = AnyAddress("tb1qcj2vfjec3c3luf9fx9vddnglhh9gawmnjan4v3", coin, "tb")
        assertEquals(address2.description(), "tb1qcj2vfjec3c3luf9fx9vddnglhh9gawmnjan4v3")
    }

    @Test
    fun testCreateWithPublicKey() {
        val coin = CoinType.BITCOIN
        val pubkey = PublicKey(ANY_ADDRESS_TEST_PUBKEY.toHexByteArray(), Curve.SECP256K1)
        val address = AnyAddress(pubkey, coin)
        assertEquals(address.description(), ANY_ADDRESS_TEST_ADDRESS)
    }

    @Test
    fun testCreateWithPublicKeyDerivation() {
        val coin = CoinType.BITCOIN
        val pubkey = PublicKey(ANY_ADDRESS_TEST_PUBKEY.toHexByteArray(), Curve.SECP256K1)
        val address1 = AnyAddress(pubkey, coin, Derivation.BITCOINSEGWIT)
        assertEquals(address1.description(), ANY_ADDRESS_TEST_ADDRESS)

        val address2 = AnyAddress(pubkey, coin, Derivation.BITCOINLEGACY)
        assertEquals(address2.description(), "1JvRfEQFv5q5qy9uTSAezH7kVQf4hqnHXx")
    }

    @Test
    fun testCreateBech32WithPublicKey() {
        val coin = CoinType.BITCOIN
        val pubkey = PublicKey(ANY_ADDRESS_TEST_PUBKEY.toHexByteArray(), Curve.SECP256K1)
        val address1 = AnyAddress(pubkey, coin, "bc")
        assertEquals(address1.description(), ANY_ADDRESS_TEST_ADDRESS)

        val address2 = AnyAddress(pubkey, coin, "tb")
        assertEquals(address2.description(), "tb1qcj2vfjec3c3luf9fx9vddnglhh9gawmnjan4v3")
    }

    @Test
    fun testIsValid() {
        val coin = CoinType.BITCOIN
        XCTAssertTrue(AnyAddress.isValid(ANY_ADDRESS_TEST_ADDRESS, coin));
        XCTAssertFalse(AnyAddress.isValid(ANY_ADDRESS_TEST_ADDRESS, CoinType.ETHEREUEM));
        XCTAssertFalse(AnyAddress.isValid("__INVALID_ADDRESS__", CoinType.ETHEREUEM));
    }

    @Test
    fun testIsValidBech32() {
        val coin = CoinType.BITCOIN
        XCTAssertTrue(AnyAddress.isValidBech32(ANY_ADDRESS_TEST_ADDRESS, coin, "bc"));
        XCTAssertFalse(AnyAddress.isValidBech32(ANY_ADDRESS_TEST_ADDRESS, coin, "tb"));
    }
}
