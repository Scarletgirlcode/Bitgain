// SPDX-License-Identifier: Apache-2.0
//
// Copyright © 2017 Trust Wallet.

//! Source code: https://github.com/solana-labs/solana/blob/a16f982169eb197fad0eb8c58c307fb069f69d8f/sdk/program/src/message/versions/mod.rs

use crate::transaction::{
    legacy, short_vec, v0, CompiledInstruction, MessageHeader, Pubkey, Signature,
};
use serde::de::{SeqAccess, Unexpected, Visitor};
use serde::ser::SerializeTuple;
use serde::{de, Deserialize, Deserializer, Serialize, Serializer};
use std::fmt;
use tw_hash::{as_byte_sequence, H256};

/// Bit mask that indicates whether a serialized message is versioned.
pub const MESSAGE_VERSION_PREFIX: u8 = 0x80;

// NOTE: Serialization-related changes must be paired with the direct read at sigverify.
/// An atomic transaction
#[derive(Debug, PartialEq, Eq, Clone, Serialize, Deserialize)]
pub struct VersionedTransaction {
    /// List of signatures
    #[serde(with = "short_vec")]
    pub signatures: Vec<Signature>,
    /// Message to sign.
    pub message: VersionedMessage,
}

impl VersionedTransaction {
    /// Fill the signatures up with zeroed signatures
    /// (same number of signatures as [`VersionedTransaction::num_required_signatures`]).
    pub fn zeroize_signatures(&mut self) {
        self.signatures = vec![Signature::default(); self.message.num_required_signatures()];
    }
}

/// Either a legacy message or a v0 message.
///
/// # Serialization
///
/// If the first bit is set, the remaining 7 bits will be used to determine
/// which message version is serialized starting from version `0`. If the first
/// is bit is not set, all bytes are used to encode the legacy `Message`
/// format.
#[derive(Debug, PartialEq, Eq, Clone)]
pub enum VersionedMessage {
    Legacy(legacy::Message),
    V0(v0::Message),
}

impl VersionedMessage {
    pub fn num_required_signatures(&self) -> usize {
        match self {
            VersionedMessage::Legacy(legacy) => legacy.header.num_required_signatures as usize,
            VersionedMessage::V0(v0) => v0.header.num_required_signatures as usize,
        }
    }

    pub fn get_account_index(&self, account_pubkey: Pubkey) -> Option<usize> {
        let account_keys = match self {
            VersionedMessage::Legacy(legacy) => &legacy.account_keys,
            VersionedMessage::V0(v0) => &v0.account_keys,
        };
        account_keys.iter().position(|pk| *pk == account_pubkey)
    }

    pub fn set_recent_blockhash(&mut self, recent_blockhash: H256) {
        match self {
            VersionedMessage::Legacy(legacy) => legacy.recent_blockhash = recent_blockhash,
            VersionedMessage::V0(v0) => v0.recent_blockhash = recent_blockhash,
        }
    }
}

impl Serialize for VersionedMessage {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match self {
            Self::Legacy(message) => {
                let mut seq = serializer.serialize_tuple(1)?;
                seq.serialize_element(message)?;
                seq.end()
            },
            Self::V0(message) => {
                let mut seq = serializer.serialize_tuple(2)?;
                seq.serialize_element(&MESSAGE_VERSION_PREFIX)?;
                seq.serialize_element(message)?;
                seq.end()
            },
        }
    }
}

enum MessagePrefix {
    Legacy(u8),
    Versioned(u8),
}

impl<'de> Deserialize<'de> for MessagePrefix {
    fn deserialize<D>(deserializer: D) -> Result<MessagePrefix, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct PrefixVisitor;

        impl<'de> Visitor<'de> for PrefixVisitor {
            type Value = MessagePrefix;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("message prefix byte")
            }

            // Serde's integer visitors bubble up to u64 so check the prefix
            // with this function instead of visit_u8. This approach is
            // necessary because serde_json directly calls visit_u64 for
            // unsigned integers.
            fn visit_u64<E: de::Error>(self, value: u64) -> Result<MessagePrefix, E> {
                if value > u8::MAX as u64 {
                    Err(de::Error::invalid_type(Unexpected::Unsigned(value), &self))?;
                }

                let byte = value as u8;
                if byte & MESSAGE_VERSION_PREFIX != 0 {
                    Ok(MessagePrefix::Versioned(byte & !MESSAGE_VERSION_PREFIX))
                } else {
                    Ok(MessagePrefix::Legacy(byte))
                }
            }
        }

        deserializer.deserialize_u8(PrefixVisitor)
    }
}

impl<'de> Deserialize<'de> for VersionedMessage {
    fn deserialize<D>(deserializer: D) -> Result<VersionedMessage, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct MessageVisitor;

        impl<'de> Visitor<'de> for MessageVisitor {
            type Value = VersionedMessage;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("message bytes")
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<VersionedMessage, A::Error>
            where
                A: SeqAccess<'de>,
            {
                let prefix: MessagePrefix = seq
                    .next_element()?
                    .ok_or_else(|| de::Error::invalid_length(0, &self))?;

                match prefix {
                    MessagePrefix::Legacy(num_required_signatures) => {
                        // The remaining fields of the legacy Message struct after the first byte.
                        #[derive(Serialize, Deserialize)]
                        struct RemainingLegacyMessage {
                            pub num_readonly_signed_accounts: u8,
                            pub num_readonly_unsigned_accounts: u8,
                            #[serde(with = "short_vec")]
                            pub account_keys: Vec<Pubkey>,
                            #[serde(with = "as_byte_sequence")]
                            pub recent_blockhash: H256,
                            #[serde(with = "short_vec")]
                            pub instructions: Vec<CompiledInstruction>,
                        }

                        let message: RemainingLegacyMessage =
                            seq.next_element()?.ok_or_else(|| {
                                // will never happen since tuple length is always 2
                                de::Error::invalid_length(1, &self)
                            })?;

                        Ok(VersionedMessage::Legacy(legacy::Message {
                            header: MessageHeader {
                                num_required_signatures,
                                num_readonly_signed_accounts: message.num_readonly_signed_accounts,
                                num_readonly_unsigned_accounts: message
                                    .num_readonly_unsigned_accounts,
                            },
                            account_keys: message.account_keys,
                            recent_blockhash: message.recent_blockhash,
                            instructions: message.instructions,
                        }))
                    },
                    MessagePrefix::Versioned(version) => {
                        match version {
                            0 => {
                                Ok(VersionedMessage::V0(seq.next_element()?.ok_or_else(
                                    || {
                                        // will never happen since tuple length is always 2
                                        de::Error::invalid_length(1, &self)
                                    },
                                )?))
                            },
                            127 => {
                                // 0xff is used as the first byte of the off-chain messages
                                // which corresponds to version 127 of the versioned messages.
                                // This explicit check is added to prevent the usage of version 127
                                // in the runtime as a valid transaction.
                                Err(de::Error::custom("off-chain messages are not accepted"))
                            },
                            _ => Err(de::Error::invalid_value(
                                de::Unexpected::Unsigned(version as u64),
                                &"a valid transaction message version",
                            )),
                        }
                    },
                }
            }
        }

        deserializer.deserialize_tuple(2, MessageVisitor)
    }
}
