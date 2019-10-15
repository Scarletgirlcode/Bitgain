package com.trustwallet.core.app.blockchains.nuls

import com.trustwallet.core.app.utils.toHex
import com.trustwallet.core.app.utils.toHexByteArray
import org.junit.Assert.assertEquals
import org.junit.Test
import wallet.core.jni.NULSAddress
import wallet.core.jni.PrivateKey
import wallet.core.jni.PublicKey
import wallet.core.jni.PublicKeyType

class TestNULSAddress {

    init {
        System.loadLibrary("TrustWalletCore")
    }

    @Test
    fun testAddress() {
        val key = PrivateKey("a1269039e4ffdf43687852d7247a295f0b5bc55e6dda031cffaa3295ca0a9d7a".toHexByteArray())
        val pubkey = key.publicKeyEd25519
        val address = NULSAddress(pubkey)
        val expected = NULSAddress("NULSd6HghWa4CN5qdxqMwYVikQxRZyj57Jn4L")

        assertEquals(address.description(), expected.description())
    }
}
