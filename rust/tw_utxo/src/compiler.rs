use crate::{Error, Result};
use bitcoin::blockdata::locktime::absolute::{Height, LockTime, Time};
use bitcoin::consensus::Encodable;
use bitcoin::hashes::Hash;
use bitcoin::sighash::{EcdsaSighashType, Prevouts, SighashCache, TapSighashType};
use bitcoin::taproot::TapLeafHash;
use bitcoin::{OutPoint, Script, ScriptBuf, Sequence, Transaction, TxIn, TxOut, Txid, Witness};
use std::borrow::Cow;
use std::marker::PhantomData;
use tw_proto::Utxo::Proto::{self, SighashType};

type ProtoLockTimeVariant = Proto::mod_LockTime::OneOfvariant;
type ProtoSigningMethod = Proto::SigningMethod;

pub trait UtxoContext {
    type SigningInput<'a>;
    type SigningOutput;
    type PreSigningOutput;
}

pub struct StandardBitcoinContext;

impl UtxoContext for StandardBitcoinContext {
    type SigningInput<'a> = Proto::SigningInput<'a>;
    type SigningOutput = Proto::SigningInput<'static>;
    type PreSigningOutput = Proto::SigningInput<'static>;
}

pub struct Compiler<Context: UtxoContext> {
    _phantom: PhantomData<Context>,
}

impl Compiler<StandardBitcoinContext> {
    #[inline]
    pub fn preimage_hashes(proto: Proto::SigningInput<'_>) -> Proto::PreSigningOutput<'static> {
        Self::preimage_hashes_impl(proto)
            .or_else(|err| {
                std::result::Result::<_, ()>::Ok(Proto::PreSigningOutput {
                    error: err.into(),
                    ..Default::default()
                })
            })
            .expect("did not convert error value")
    }

    #[inline]
    pub fn compile(proto: Proto::PreSerialization<'_>) -> Proto::SerializedTransaction<'static> {
        Self::compile_impl(proto)
            .or_else(|err| {
                std::result::Result::<_, ()>::Ok(Proto::SerializedTransaction {
                    error: err.into(),
                    ..Default::default()
                })
            })
            .expect("did not convert error value")
    }

    fn preimage_hashes_impl(
        mut proto: Proto::SigningInput<'_>,
    ) -> Result<Proto::PreSigningOutput<'static>> {
        // Calculate total outputs amount, based on it we can determine how many inputs to select.
        let mut total_input = 0;
        let total_output: u64 = proto.outputs.iter().map(|output| output.value).sum();

        let mut remaining = total_output;

        // Only use the necessariy amount of inputs to cover `total_output`, any
        // other input gets dropped.
        let proto_inputs = std::mem::take(&mut proto.inputs);
        let selected: Vec<Proto::TxIn> = proto_inputs
            .into_iter()
            .take_while(|input| {
                if remaining == 0 {
                    return false;
                }

                total_input += input.amount;
                remaining = remaining.saturating_sub(input.amount);

                true
            })
            .map(|input| Proto::TxIn {
                txid: input.txid.to_vec().into(),
                script_pubkey: input.script_pubkey.to_vec().into(),
                leaf_hash: input.leaf_hash.to_vec().into(),
                ..input
            })
            .collect();

        // Insufficient input amount.
        if remaining != 0 {
            // Return error.
            todo!()
        }

        // Update protobuf structure with selected inputs.
        proto.inputs = selected.clone();

        // Calculate the total input weight projection.
        let input_weight: u64 = proto
            .inputs
            .iter()
            .map(|input| input.weight_projection)
            .sum();

        // Convert Protobuf structure to `bitcoin` crate native transaction.
        let tx = convert_proto_to_tx(&proto)?;

        // Calculate the full weight projection (base weight + input & output weight).
        let weight_projection = tx.weight().to_wu() + input_weight;
        let fee_projection = weight_projection * proto.weight_base;

        // The amount to be returned.
        let change_amount = total_input - fee_projection;

        // Update the passed on protobuf structure by adding a change output
        // (return to sender)
        /*
        proto.outputs.push(Proto::TxOut {
            value: change_amount,
            script_pubkey: proto.change_script_pubkey.clone(),
        });
        */

        // Convert *updated* Protobuf structure to `bitcoin` crate native
        // transaction.
        let tx = convert_proto_to_tx(&proto)?;
        let mut cache = SighashCache::new(&tx);

        let mut sighashes: Vec<(Vec<u8>, ProtoSigningMethod)> = vec![];

        for (index, input) in proto.inputs.iter().enumerate() {
            match input.signing_method {
                // Use the legacy hashing mechanism (e.g. P2SH, P2PK, P2PKH).
                ProtoSigningMethod::Legacy => {
                    let script_pubkey = Script::from_bytes(input.script_pubkey.as_ref());
                    let sighash_type = if let SighashType::UseDefault = input.sighash_type {
                        EcdsaSighashType::All
                    } else {
                        EcdsaSighashType::from_consensus(input.sighash_type as u32)
                    };

                    let sighash =
                        cache.legacy_signature_hash(index, script_pubkey, sighash_type.to_u32())?;

                    sighashes.push((sighash.as_byte_array().to_vec(), ProtoSigningMethod::Legacy));
                },
                // Use the Segwit hashing mechanism (e.g. P2WSH, P2WPKH).
                ProtoSigningMethod::Segwit => {
                    let script_pubkey = ScriptBuf::from_bytes(input.script_pubkey.to_vec());
                    let sighash_type = if let SighashType::UseDefault = input.sighash_type {
                        EcdsaSighashType::All
                    } else {
                        EcdsaSighashType::from_consensus(input.sighash_type as u32)
                    };

                    let sighash = cache.segwit_signature_hash(
                        index,
                        script_pubkey
                            .p2wpkh_script_code()
                            .as_ref()
                            .ok_or(Error::from(Proto::Error::Error_invalid_wpkh_script_pubkey))?,
                        input.amount,
                        sighash_type,
                    )?;

                    sighashes.push((sighash.as_byte_array().to_vec(), ProtoSigningMethod::Segwit));
                },
                // Use the Taproot hashing mechanism (e.g. P2TR key-path/script-path)
                ProtoSigningMethod::Taproot => {
                    let leaf_hash = if input.leaf_hash.is_empty() {
                        None
                    } else {
                        Some((
                            TapLeafHash::from_slice(input.leaf_hash.as_ref())
                                .map_err(|_| Error::from(Proto::Error::Error_invalid_leaf_hash))?,
                            // TODO: We might want to make this configurable?.
                            0xFFFFFFFF,
                        ))
                    };

                    // Note that `input.sighash_type = 0` is handled by the underlying library.
                    let sighash_type = TapSighashType::from_consensus_u8(input.sighash_type as u8)
                        .map_err(|_| Error::from(Proto::Error::Error_invalid_sighash_type))?;

                    // This owner only exists to avoid running into lifetime
                    // issues related to `Prevouts::All(&[T])`.
                    let _owner;

                    let prevouts = if input.one_prevout {
                        Prevouts::One(
                            index,
                            TxOut {
                                value: input.amount,
                                script_pubkey: ScriptBuf::from_bytes(input.script_pubkey.to_vec()),
                            },
                        )
                    } else {
                        _owner = Some(
                            proto
                                .inputs
                                .iter()
                                .map(|i| TxOut {
                                    value: i.amount,
                                    script_pubkey: ScriptBuf::from_bytes(i.script_pubkey.to_vec()),
                                })
                                .collect::<Vec<TxOut>>(),
                        );

                        Prevouts::All(_owner.as_ref().expect("_owner not initialized"))
                    };

                    dbg!(&prevouts);

                    let sighash = cache.taproot_signature_hash(
                        index,
                        &prevouts,
                        None,
                        leaf_hash,
                        sighash_type,
                    )?;

                    sighashes.push((
                        sighash.as_byte_array().to_vec(),
                        ProtoSigningMethod::Taproot,
                    ));
                },
            }
        }

        Ok(Proto::PreSigningOutput {
            error: Proto::Error::OK,
            inputs: selected,
            sighashes: sighashes
                .into_iter()
                .map(|(sighash, method)| Proto::Sighash {
                    sighash: sighash.into(),
                    signing_method: method.into(),
                })
                .collect(),
            weight_projection,
        })
    }

    fn compile_impl(
        proto: Proto::PreSerialization<'_>,
    ) -> Result<Proto::SerializedTransaction<'static>> {
        let mut tx = Transaction {
            version: proto.version,
            lock_time: lock_time_from_proto(&proto.lock_time)?,
            input: vec![],
            output: vec![],
        };

        for txin in &proto.inputs {
            let txid = Txid::from_slice(txin.txid.as_ref())
                .map_err(|_| Error::from(Proto::Error::Error_invalid_txid))?;
            let vout = txin.vout;
            let sequence = Sequence::from_consensus(txin.sequence);
            let script_sig = ScriptBuf::from_bytes(txin.script_sig.to_vec());
            let witness = Witness::from_slice(
                &txin
                    .witness_items
                    .iter()
                    .map(|s| s.as_ref())
                    .collect::<Vec<&[u8]>>(),
            );

            tx.input.push(TxIn {
                previous_output: OutPoint { txid, vout },
                script_sig,
                sequence,
                witness,
            });
        }

        for txout in &proto.outputs {
            tx.output.push(TxOut {
                value: txout.value,
                script_pubkey: ScriptBuf::from_bytes(txout.script_pubkey.to_vec()),
            });
        }

        // Encode the transaction.
        let mut buffer = vec![];
        tx.consensus_encode(&mut buffer)
            .map_err(|_| Error::from(Proto::Error::Error_failed_encoding))?;

        Ok(Proto::SerializedTransaction {
            error: Proto::Error::OK,
            encoded: buffer.into(),
            weight: tx.weight().to_wu(),
            fee: tx.weight().to_vbytes_ceil() * proto.weight_base,
        })
    }
}

fn convert_proto_to_tx<'a>(proto: &'a Proto::SigningInput<'a>) -> Result<Transaction> {
    let mut tx = Transaction {
        version: proto.version,
        lock_time: lock_time_from_proto(&proto.lock_time)?,
        input: vec![],
        output: vec![],
    };

    for txin in &proto.inputs {
        let txid = Txid::from_slice(txin.txid.as_ref())
            .map_err(|_| Error::from(Proto::Error::Error_invalid_txid))?;
        let vout = txin.vout;

        tx.input.push(TxIn {
            previous_output: OutPoint { txid, vout },
            script_sig: ScriptBuf::new(),
            // TODO: This is actually important for signing, add as field in Utxo.
            sequence: Sequence::default(),
            witness: Witness::new(),
        });
    }

    for txout in &proto.outputs {
        tx.output.push(TxOut {
            value: txout.value,
            script_pubkey: ScriptBuf::from_bytes(txout.script_pubkey.to_vec()),
        });
    }

    Ok(tx)
}

// Convenience function to retreive the lock time. If none is provided, the
// default lock time is used (immediately spendable).
fn lock_time_from_proto(proto: &Option<Proto::LockTime>) -> Result<LockTime> {
    let lock_time = if let Some(lock_time) = proto {
        match lock_time.variant {
            ProtoLockTimeVariant::blocks(block) => LockTime::Blocks(
                Height::from_consensus(block)
                    .map_err(|_| Error::from(Proto::Error::Error_invalid_lock_time))?,
            ),
            ProtoLockTimeVariant::seconds(secs) => LockTime::Seconds(
                Time::from_consensus(secs)
                    .map_err(|_| Error::from(Proto::Error::Error_invalid_lock_time))?,
            ),
            ProtoLockTimeVariant::None => LockTime::Blocks(
                Height::from_consensus(0)
                    .map_err(|_| Error::from(Proto::Error::Error_invalid_lock_time))?,
            ),
        }
    } else {
        LockTime::Blocks(
            Height::from_consensus(0)
                .map_err(|_| Error::from(Proto::Error::Error_invalid_lock_time))?,
        )
    };

    Ok(lock_time)
}
