use std::collections::HashMap;
use std::convert::identity;
use std::iter::{repeat, Iterator};
use std::str::FromStr;

use lazy_static::lazy_static;

use tw_number::U256;
use tw_proto::Polkadot::Proto;
use tw_proto::Polkadot::Proto::mod_Balance::{
    AssetTransfer, BatchAssetTransfer, BatchTransfer, OneOfmessage_oneof as BalanceVariant,
    Transfer,
};
use tw_proto::Polkadot::Proto::mod_CallIndices::OneOfvariant as CallIndicesVariant;
use tw_proto::Polkadot::Proto::mod_Identity::{
    AddAuthorization, JoinIdentityAsKey, OneOfmessage_oneof as PolymeshIdentityVariant,
};
use tw_proto::Polkadot::Proto::mod_PolymeshCall::OneOfmessage_oneof as PolymeshVariant;
use tw_proto::Polkadot::Proto::mod_SigningInput::OneOfmessage_oneof as SigningVariant;
use tw_proto::Polkadot::Proto::mod_Staking::{
    Bond, BondAndNominate, BondExtra, Chill, ChillAndUnbond, Nominate,
    OneOfmessage_oneof as StakingVariant, Rebond, Unbond, WithdrawUnbonded,
};
use tw_proto::Polkadot::Proto::{Balance, PolymeshCall, Staking};
use tw_ss58_address::{NetworkId, SS58Address};

use crate::scale::{Compact, Raw, RawIter, ToScale};

const POLKADOT_MULTI_ADDRESS_SPEC: u32 = 28;
const KUSAMA_MULTI_ADDRESS_SPEC: u32 = 2028;

// Common calls
const BALANCE_TRANSFER: &str = "Balances.transfer";
const STAKING_BOND: &str = "Staking.bond";
const STAKING_BOND_EXTRA: &str = "Staking.bond_extra";
const STAKING_CHILL: &str = "Staking.chill";
const STAKING_NOMINATE: &str = "Staking.nominate";
const STAKING_REBOND: &str = "Staking.rebond";
const STAKING_UNBOND: &str = "Staking.unbond";
const STAKING_WITHDRAW_UNBONDED: &str = "Staking.withdraw_unbonded";
const UTILITY_BATCH: &str = "Utility.batch_all";

// Non-existent calls on Polkadot and Kusama chains
const ASSETS_TRANSFER: &str = "Assets.transfer";
const JOIN_IDENTITY_AS_KEY: &str = "Identity.join_identity_as_key";
const IDENTITY_ADD_AUTHORIZATION: &str = "Identity.add_authorization";

type CallIndex = (u8, u8);
type CallIndicesTable = HashMap<&'static str, CallIndex>;

macro_rules! call_indices {
    ($($chain:expr => { $($name:expr => ($($value:expr),*) $(,)?)* } $(,)? )*) => {
        [
            $((
                $chain, std::collections::HashMap::from_iter(
                    [$(($name, ($($value as u8),+))),+]
                )
            )),+
        ].into_iter().collect()
    }
}

lazy_static! {
    static ref CALL_INDICES_BY_NETWORK: HashMap<NetworkId, CallIndicesTable> = call_indices! {
        NetworkId::POLKADOT => {
            BALANCE_TRANSFER          => (0x05, 0x00),
            STAKING_BOND              => (0x07, 0x00),
            STAKING_BOND_EXTRA        => (0x07, 0x01),
            STAKING_CHILL             => (0x07, 0x06),
            STAKING_NOMINATE          => (0x07, 0x05),
            STAKING_REBOND            => (0x07, 0x13),
            STAKING_UNBOND            => (0x07, 0x02),
            STAKING_WITHDRAW_UNBONDED => (0x07, 0x03),
            UTILITY_BATCH             => (0x1a, 0x02),
        },
        NetworkId::KUSAMA => {
            BALANCE_TRANSFER          => (0x04, 0x00),
            STAKING_BOND              => (0x06, 0x00),
            STAKING_BOND_EXTRA        => (0x06, 0x01),
            STAKING_CHILL             => (0x06, 0x06),
            STAKING_NOMINATE          => (0x06, 0x05),
            STAKING_REBOND            => (0x06, 0x13),
            STAKING_UNBOND            => (0x06, 0x02),
            STAKING_WITHDRAW_UNBONDED => (0x06, 0x03),
            UTILITY_BATCH             => (0x18, 0x02),
        }
    };
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub enum EncodeError {
    InvalidNetworkId,
    MissingCallIndicesTable,
    InvalidCallIndex,
    InvalidAddress,
    InvalidValue,
}

type EncodeResult<T> = Result<T, EncodeError>;

impl ToScale for SS58Address {
    fn to_scale_into(&self, out: &mut Vec<u8>) {
        Raw(self.key_bytes()).to_scale_into(out)
    }
}

#[derive(Debug, Clone, Default)]
pub struct ExtrinsicEncoder {
    encoded: Vec<u8>,
}

impl ExtrinsicEncoder {
    pub fn new() -> Self {
        Default::default()
    }

    fn from_encoded(encoded: Vec<u8>) -> Self {
        Self { encoded }
    }

    pub fn add(self, t: impl ToScale) -> Self {
        let mut encoded = self.encoded;
        t.to_scale_into(&mut encoded);
        Self::from_encoded(encoded)
    }

    pub fn add_mut(&mut self, t: impl ToScale) -> &mut Self {
        t.to_scale_into(&mut self.encoded);
        self
    }

    pub fn finalize(self) -> Vec<u8> {
        self.encoded
    }
}

// `Extrinsic` is (for now) just a lightweight wrapper over the actual protobuf object.
// In the future, we will refine the latter to let the caller specify arbitrary extrinsics.

#[derive(Debug, Clone)]
pub struct Extrinsic<'a> {
    inner: Proto::SigningInput<'a>,
}

impl<'a> Extrinsic<'a> {
    pub fn from_input(input: Proto::SigningInput<'a>) -> Self {
        Self { inner: input }
    }

    fn get_call_index_for_network(network: NetworkId, key: &str) -> EncodeResult<(u8, u8)> {
        CALL_INDICES_BY_NETWORK
            .get(&network)
            .and_then(|table| table.get(key))
            .cloned()
            .ok_or(EncodeError::MissingCallIndicesTable)
    }

    fn get_custom_call_index(civ: &Option<Proto::CallIndices>) -> EncodeResult<(u8, u8)> {
        if let Some(CallIndicesVariant::custom(c)) = civ.as_ref().map(|i| &i.variant) {
            if c.module_index > 0xff || c.method_index > 0xff {
                return Err(EncodeError::InvalidCallIndex);
            }
            return Ok((c.module_index as u8, c.method_index as u8));
        }
        Err(EncodeError::MissingCallIndicesTable)
    }

    fn get_custom_call_index_or_network(
        &self,
        key: &str,
        civ: &Option<Proto::CallIndices>,
    ) -> EncodeResult<(u8, u8)> {
        Self::get_custom_call_index(civ).or_else(|_| {
            let network = NetworkId::try_from(self.inner.network as u16)
                .map_err(|_| EncodeError::InvalidNetworkId)?;

            Self::get_call_index_for_network(network, key)
        })
    }

    fn should_encode_raw_account(&self) -> bool {
        if self.inner.multi_address {
            return false;
        }

        let (network, spec) = (
            NetworkId::try_from(self.inner.network as u16),
            self.inner.spec_version,
        );

        match (network, spec) {
            (Ok(NetworkId::POLKADOT), _) if spec >= POLKADOT_MULTI_ADDRESS_SPEC => false,
            (Ok(NetworkId::KUSAMA), _) if spec > KUSAMA_MULTI_ADDRESS_SPEC => false,
            _ => true,
        }
    }

    fn encode_transfer(&self, t: &Transfer) -> EncodeResult<Vec<u8>> {
        let call_index =
            self.get_custom_call_index_or_network(BALANCE_TRANSFER, &t.call_indices)?;
        let address =
            SS58Address::from_str(&t.to_address).map_err(|_| EncodeError::InvalidAddress)?;
        let value =
            U256::from_little_endian_slice(&t.value).map_err(|_| EncodeError::InvalidValue)?;

        let mut encoder = ExtrinsicEncoder::new();
        encoder.add_mut(call_index);
        if !self.should_encode_raw_account() {
            encoder.add_mut(0x00u8);
        }
        encoder.add_mut(address);
        encoder.add_mut(Compact(value));

        // Encode memo if present, padding it to 32 bytes
        if !t.memo.is_empty() {
            encoder.add_mut(0x01u8);
            encoder.add_mut(Raw(t.memo.as_bytes()));
            if t.memo.len() < 32 {
                encoder.add_mut(RawIter(repeat(0x00).take(32 - t.memo.len())));
            }
        }

        Ok(encoder.finalize())
    }

    fn encode_asset_transfer(&self, at: &AssetTransfer) -> EncodeResult<Vec<u8>> {
        let call_index =
            self.get_custom_call_index_or_network(ASSETS_TRANSFER, &at.call_indices)?;
        let address =
            SS58Address::from_str(&at.to_address).map_err(|_| EncodeError::InvalidAddress)?;
        let value =
            U256::from_little_endian_slice(&at.value).map_err(|_| EncodeError::InvalidValue)?;

        let mut encoder = ExtrinsicEncoder::new().add(call_index);

        // Encode asset ID if not native token
        if at.asset_id != 0 {
            encoder.add_mut(Compact(at.asset_id));
        }

        if !self.should_encode_raw_account() {
            encoder.add_mut(0x00u8);
        }

        let data = encoder.add(address).add(Compact(value)).finalize();

        Ok(data)
    }

    fn encode_batch(
        &self,
        encoded_calls: &[Vec<u8>],
        call_indices: &Option<Proto::CallIndices>,
    ) -> EncodeResult<Vec<u8>> {
        let call_index = self.get_custom_call_index_or_network(UTILITY_BATCH, call_indices)?;
        let encoder = ExtrinsicEncoder::new()
            .add(call_index)
            .add(Compact(encoded_calls.len()));

        let data = encoded_calls
            .iter()
            .fold(encoder, |encoder, call| encoder.add(Raw(call)))
            .finalize();

        Ok(data)
    }

    fn encode_batch_transfer(&self, bt: &BatchTransfer) -> EncodeResult<Vec<u8>> {
        let transfers = bt
            .transfers
            .iter()
            .map(|t| self.encode_transfer(t))
            .collect::<EncodeResult<Vec<_>>>()?;

        self.encode_batch(&transfers, &bt.call_indices)
    }

    fn encode_batch_asset_transfer(&self, bat: &BatchAssetTransfer) -> EncodeResult<Vec<u8>> {
        let transfers = bat
            .transfers
            .iter()
            .map(|t| self.encode_asset_transfer(t))
            .collect::<EncodeResult<Vec<_>>>()?;

        self.encode_batch(&transfers, &bat.call_indices)
    }

    fn encode_balance_call(&self, b: &Balance) -> EncodeResult<Vec<u8>> {
        match &b.message_oneof {
            BalanceVariant::transfer(t) => self.encode_transfer(t),
            BalanceVariant::batchTransfer(bt) => self.encode_batch_transfer(bt),
            BalanceVariant::asset_transfer(at) => self.encode_asset_transfer(at),
            BalanceVariant::batch_asset_transfer(bat) => self.encode_batch_asset_transfer(bat),
            BalanceVariant::None => Ok(Vec::new()),
        }
    }

    fn encode_staking_bond(&self, b: &Bond) -> EncodeResult<Vec<u8>> {
        let call_index = self.get_custom_call_index_or_network(STAKING_BOND, &b.call_indices)?;
        let value =
            U256::from_little_endian_slice(&b.value).map_err(|_| EncodeError::InvalidValue)?;

        let mut encoder = ExtrinsicEncoder::new().add(call_index);

        if !b.controller.is_empty() {
            // TODO: check address network ?
            let address =
                SS58Address::from_str(&b.controller).map_err(|_| EncodeError::InvalidAddress)?;
            if !self.should_encode_raw_account() {
                encoder.add_mut(0x00u8);
            }
            encoder.add_mut(address);
        }

        let data = encoder
            .add(Compact(value))
            .add(b.reward_destination as u8)
            .finalize();

        Ok(data)
    }

    fn encode_staking_bond_and_nominate(&self, ban: &BondAndNominate) -> EncodeResult<Vec<u8>> {
        // Encode a bond call
        let first = self.encode_staking_call(&Staking {
            message_oneof: StakingVariant::bond(Bond {
                controller: ban.controller.clone(),
                value: ban.value.clone(),
                reward_destination: ban.reward_destination,
                call_indices: ban.call_indices.clone(),
            }),
        })?;

        // Encode a nominate call
        let second = self.encode_staking_call(&Staking {
            message_oneof: StakingVariant::nominate(Nominate {
                nominators: ban.nominators.clone(),
                call_indices: ban.call_indices.clone(),
            }),
        })?;

        // Encode both calls as batched
        self.encode_batch(&[first, second], &ban.call_indices)
    }

    fn encode_staking_bond_extra(&self, be: &BondExtra) -> EncodeResult<Vec<u8>> {
        let call_index =
            self.get_custom_call_index_or_network(STAKING_BOND_EXTRA, &be.call_indices)?;
        let value =
            U256::from_little_endian_slice(&be.value).map_err(|_| EncodeError::InvalidValue)?;

        let data = ExtrinsicEncoder::new()
            .add(call_index)
            .add(Compact(value))
            .finalize();

        Ok(data)
    }

    fn encode_staking_unbond(&self, u: &Unbond) -> EncodeResult<Vec<u8>> {
        let call_index = self.get_custom_call_index_or_network(STAKING_UNBOND, &u.call_indices)?;
        let value =
            U256::from_little_endian_slice(&u.value).map_err(|_| EncodeError::InvalidValue)?;

        let data = ExtrinsicEncoder::new()
            .add(call_index)
            .add(Compact(value))
            .finalize();

        Ok(data)
    }

    fn encode_staking_rebond(&self, u: &Rebond) -> EncodeResult<Vec<u8>> {
        let call_index = self.get_custom_call_index_or_network(STAKING_REBOND, &u.call_indices)?;
        let value =
            U256::from_little_endian_slice(&u.value).map_err(|_| EncodeError::InvalidValue)?;

        let data = ExtrinsicEncoder::new()
            .add(call_index)
            .add(Compact(value))
            .finalize();

        Ok(data)
    }

    fn encode_staking_withdraw_unbonded(&self, wu: &WithdrawUnbonded) -> EncodeResult<Vec<u8>> {
        let call_index =
            self.get_custom_call_index_or_network(STAKING_WITHDRAW_UNBONDED, &wu.call_indices)?;

        let data = ExtrinsicEncoder::new()
            .add(call_index)
            .add(wu.slashing_spans as u32)
            .finalize();

        Ok(data)
    }

    fn encode_staking_nominate(&self, n: &Nominate) -> EncodeResult<Vec<u8>> {
        let mut data = Vec::new();

        // Encode call index
        let call_index =
            self.get_custom_call_index_or_network(STAKING_NOMINATE, &n.call_indices)?;
        call_index.to_scale_into(&mut data);

        // Encode account IDs for nominators
        Compact(n.nominators.len()).to_scale_into(&mut data);
        let raw = self.should_encode_raw_account();
        n.nominators
            .iter()
            .map(|s| {
                let addr = SS58Address::from_str(s).map_err(|_| EncodeError::InvalidAddress)?;
                if !raw {
                    data.push(0x00);
                }
                addr.to_scale_into(&mut data);
                Ok(())
            })
            .try_for_each(identity)?;

        Ok(data)
    }

    fn encode_staking_chill(&self, c: &Chill) -> EncodeResult<Vec<u8>> {
        // Encode call index
        let call_index = self.get_custom_call_index_or_network(STAKING_CHILL, &c.call_indices)?;

        Ok(call_index.to_scale())
    }

    fn encode_staking_chill_and_unbond(&self, cau: &ChillAndUnbond) -> EncodeResult<Vec<u8>> {
        let first = self.encode_staking_call(&Staking {
            message_oneof: StakingVariant::chill(Chill {
                call_indices: cau.call_indices.clone(),
            }),
        })?;

        let second = self.encode_staking_call(&Staking {
            message_oneof: StakingVariant::unbond(Unbond {
                value: cau.value.clone(),
                call_indices: cau.call_indices.clone(),
            }),
        })?;

        // Encode both calls as batched
        self.encode_batch(&[first, second], &cau.call_indices)
    }

    fn encode_staking_call(&self, s: &Staking) -> EncodeResult<Vec<u8>> {
        match &s.message_oneof {
            StakingVariant::bond(b) => self.encode_staking_bond(b),
            StakingVariant::bond_and_nominate(ban) => self.encode_staking_bond_and_nominate(ban),
            StakingVariant::bond_extra(be) => self.encode_staking_bond_extra(be),
            StakingVariant::unbond(u) => self.encode_staking_unbond(u),
            StakingVariant::withdraw_unbonded(wu) => self.encode_staking_withdraw_unbonded(wu),
            StakingVariant::nominate(n) => self.encode_staking_nominate(n),
            StakingVariant::chill(c) => self.encode_staking_chill(c),
            StakingVariant::chill_and_unbond(cau) => self.encode_staking_chill_and_unbond(cau),
            StakingVariant::rebond(r) => self.encode_staking_rebond(r),
            StakingVariant::None => Ok(Vec::new()),
        }
    }

    fn encode_polymesh_join_identity_as_key(&self, j: &JoinIdentityAsKey) -> EncodeResult<Vec<u8>> {
        let mut data = Vec::new();

        // Encode call index
        let call_index =
            self.get_custom_call_index_or_network(JOIN_IDENTITY_AS_KEY, &j.call_indices)?;
        call_index.to_scale_into(&mut data);

        // Encode auth ID
        j.auth_id.to_scale_into(&mut data);

        Ok(data)
    }

    fn encode_polymesh_add_authorization(&self, a: &AddAuthorization) -> EncodeResult<Vec<u8>> {
        let mut data = Vec::new();

        // Encode call index
        let call_index =
            self.get_custom_call_index_or_network(IDENTITY_ADD_AUTHORIZATION, &a.call_indices)?;
        call_index.to_scale_into(&mut data);

        // Encode target
        data.push(0x01);
        let address = SS58Address::from_str(&a.target).map_err(|_| EncodeError::InvalidAddress)?;
        address.to_scale_into(&mut data);

        // Encode join identity
        data.push(0x05);

        if let Some(auth_data) = &a.data {
            if let Some(asset) = &auth_data.asset {
                data.push(0x01);
                Raw(&asset.data).to_scale_into(&mut data);
            } else {
                data.push(0x00);
            }

            if let Some(extrinsic) = &auth_data.extrinsic {
                data.push(0x01);
                Raw(&extrinsic.data).to_scale_into(&mut data);
            } else {
                data.push(0x00);
            }

            if let Some(portfolio) = &auth_data.portfolio {
                data.push(0x01);
                Raw(&portfolio.data).to_scale_into(&mut data);
            } else {
                data.push(0x00);
            }
        } else {
            // Mark everything as authorized (asset, extrinsic, portfolio)
            (0x01u8, 0x00u8).to_scale_into(&mut data);
            (0x01u8, 0x00u8).to_scale_into(&mut data);
            (0x01u8, 0x00u8).to_scale_into(&mut data);
        }

        Compact(a.expiry).to_scale_into(&mut data);

        Ok(data)
    }

    fn encode_polymesh_identity(&self, i: &Proto::Identity) -> EncodeResult<Vec<u8>> {
        match &i.message_oneof {
            PolymeshIdentityVariant::join_identity_as_key(j) => {
                self.encode_polymesh_join_identity_as_key(j)
            },
            PolymeshIdentityVariant::add_authorization(a) => {
                self.encode_polymesh_add_authorization(a)
            },
            PolymeshIdentityVariant::None => Ok(Vec::new()),
        }
    }

    fn encode_polymesh_call(&self, p: &PolymeshCall) -> EncodeResult<Vec<u8>> {
        match &p.message_oneof {
            PolymeshVariant::identity_call(i) => self.encode_polymesh_identity(i),
            PolymeshVariant::None => Ok(Vec::new()),
        }
    }

    pub fn encode_call(&self) -> EncodeResult<Vec<u8>> {
        match &self.inner.message_oneof {
            SigningVariant::balance_call(b) => self.encode_balance_call(b),
            SigningVariant::staking_call(s) => self.encode_staking_call(s),
            SigningVariant::polymesh_call(p) => self.encode_polymesh_call(p),
            SigningVariant::None => Ok(Vec::new()),
        }
    }
}
