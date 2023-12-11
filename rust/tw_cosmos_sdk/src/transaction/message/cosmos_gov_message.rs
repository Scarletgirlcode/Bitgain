// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

use crate::address::CosmosAddress;
use crate::proto::cosmos;
use crate::transaction::message::{CosmosMessage, ProtobufMessage};
use tw_coin_entry::error::SigningResult;
use tw_proto::to_any;

pub enum VoteOption {
    Unspecified,
    Yes,
    Abstain,
    No,
    NoWithVeto,
}

pub struct VoteMessage<Address: CosmosAddress> {
    pub proposal_id: u64,
    pub voter: Address,
    pub option: VoteOption,
}

impl<Address: CosmosAddress> CosmosMessage for VoteMessage<Address> {
    fn to_proto(&self) -> SigningResult<ProtobufMessage> {
        use cosmos::gov::v1beta1::VoteOption as ProtoVoteOption;

        let option = match self.option {
            VoteOption::Unspecified => ProtoVoteOption::VOTE_OPTION_UNSPECIFIED,
            VoteOption::Yes => ProtoVoteOption::VOTE_OPTION_YES,
            VoteOption::Abstain => ProtoVoteOption::VOTE_OPTION_ABSTAIN,
            VoteOption::No => ProtoVoteOption::VOTE_OPTION_NO,
            VoteOption::NoWithVeto => ProtoVoteOption::VOTE_OPTION_NO_WITH_VETO,
        };

        let proto_msg = cosmos::gov::v1beta1::MsgVote {
            proposal_id: self.proposal_id,
            voter: self.voter.to_string(),
            option,
        };
        Ok(to_any(&proto_msg))
    }
}
