// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::ed25519::public::PublicKey;
use crate::ed25519::signature::Signature;
use crate::ed25519::Hasher512;
use crate::traits::VerifyingKeyTrait;
use crate::Error;
use std::ops::Range;
use tw_encoding::hex;
use tw_hash::H256;
use tw_misc::traits::ToBytesVec;

pub struct ExtendedPublicKey<H: Hasher512> {
    key: ExtendedPublicPart<H>,
    second_key: ExtendedPublicPart<H>,
}

/// cbindgen:ignore
impl<H: Hasher512> ExtendedPublicKey<H> {
    pub const LEN: usize = ExtendedPublicPart::<H>::LEN * 2;
    const KEY_RANGE: Range<usize> = 0..ExtendedPublicPart::<H>::LEN;
    const SECOND_KEY_RANGE: Range<usize> = ExtendedPublicPart::<H>::LEN..Self::LEN;

    pub(crate) fn new(key: ExtendedPublicPart<H>, second_key: ExtendedPublicPart<H>) -> Self {
        ExtendedPublicKey { key, second_key }
    }
}

impl<H: Hasher512> VerifyingKeyTrait for ExtendedPublicKey<H> {
    type SigningMessage = Vec<u8>;
    type VerifySignature = Signature;

    fn verify(&self, signature: Self::VerifySignature, message: Self::SigningMessage) -> bool {
        self.key.public.verify(signature, message)
    }
}

impl<H: Hasher512> ToBytesVec for ExtendedPublicKey<H> {
    fn to_vec(&self) -> Vec<u8> {
        let mut res = self.key.to_vec();
        res.extend_from_slice(self.second_key.to_vec().as_slice());
        res
    }
}

impl<'a, H: Hasher512> TryFrom<&'a [u8]> for ExtendedPublicKey<H> {
    type Error = Error;

    fn try_from(bytes: &'a [u8]) -> Result<Self, Self::Error> {
        if bytes.len() != Self::LEN {
            return Err(Error::InvalidPublicKey);
        }

        let key = ExtendedPublicPart::try_from(&bytes[Self::KEY_RANGE])?;
        let second_key = ExtendedPublicPart::try_from(&bytes[Self::SECOND_KEY_RANGE])?;

        Ok(ExtendedPublicKey { key, second_key })
    }
}

impl<H: Hasher512> From<&'static str> for ExtendedPublicKey<H> {
    fn from(hex: &'static str) -> Self {
        let bytes = hex::decode(hex).expect("Expected a valid Public Key hex");
        ExtendedPublicKey::try_from(bytes.as_slice()).expect("Expected a valid Public Key")
    }
}

pub(crate) struct ExtendedPublicPart<H: Hasher512> {
    public: PublicKey<H>,
    chain_code: H256,
}

/// cbindgen:ignore
impl<H: Hasher512> ExtendedPublicPart<H> {
    const LEN: usize = PublicKey::<H>::LEN + H256::len();
    const PUBLIC_RANGE: Range<usize> = 0..32;
    const CHAIN_CODE_RANGE: Range<usize> = 32..64;

    pub(crate) fn new(public: PublicKey<H>, chain_code: H256) -> ExtendedPublicPart<H> {
        ExtendedPublicPart { public, chain_code }
    }
}

impl<'a, H: Hasher512> TryFrom<&'a [u8]> for ExtendedPublicPart<H> {
    type Error = Error;

    fn try_from(bytes: &'a [u8]) -> Result<Self, Self::Error> {
        if bytes.len() != Self::LEN {
            return Err(Error::InvalidPublicKey);
        }

        let public = PublicKey::try_from(&bytes[Self::PUBLIC_RANGE])?;
        let chain_code =
            H256::try_from(&bytes[Self::CHAIN_CODE_RANGE]).map_err(|_| Error::InvalidPublicKey)?;

        Ok(ExtendedPublicPart { public, chain_code })
    }
}

impl<H: Hasher512> ToBytesVec for ExtendedPublicPart<H> {
    fn to_vec(&self) -> Vec<u8> {
        let mut res = Vec::with_capacity(H256::len() * 2);
        res.extend_from_slice(self.public.as_slice());
        res.extend_from_slice(self.chain_code.as_slice());
        res
    }
}
