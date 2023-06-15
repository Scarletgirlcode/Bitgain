use crate::{
    brc20::{BRC20DeployInscription, BRC20MintInscription, BRC20TransferInscription, Ticker},
    keypair_from_wif, Recipient, TXOutputP2TRScriptPath, TransactionBuilder, TxInputP2PKH,
    TxInputP2TRScriptPath, TxInputP2WPKH, TxOutputP2PKH, TxOutputP2WPKH,
};
use bitcoin::{Network, PubkeyHash, PublicKey, Txid};
use std::str::FromStr;
use tw_encoding::hex;

#[test]
// WIP...
fn mainnet_brc20_transfer() {
    const TXID: &str = "";

	pub const FULL_AMOUNT: u64 = 40_000;
	pub const MINER_FEE: u64 = 3_300;

	const BRC20_TICKER: &str = "oadf";
	const BRC20_AMOUNT: usize = 20;
	pub const BRC20_INSCRIBE_AMOUNT: u64 = 7_000;
	pub const BRC20_DUST_AMOUNT: u64 = 546;

	let ticker = Ticker::new(BRC20_TICKER.to_string()).unwrap();

    let ALICE_WIF: &str = env!("ALICE_WIF");
    let BOB_WIF: &str = env!("BOB_WIF");

    let alice = keypair_from_wif(ALICE_WIF).unwrap();
    let bob = keypair_from_wif(ALICE_WIF).unwrap();

    let txid = Txid::from_str("1121ea22e37b65639a6987b274090e03eaa148dbd02769e616c924148b8fa49b").unwrap();

	// Commit transfer.
    let input = TxInputP2WPKH::builder()
        .txid(txid)
        .vout(1)
        .recipient(alice.try_into().unwrap())
        .satoshis(FULL_AMOUNT)
        .build()
        .unwrap();

    let transfer = BRC20TransferInscription::new(alice.into(), ticker.clone(), BRC20_AMOUNT).unwrap();

    let output = TXOutputP2TRScriptPath::builder()
        .recipient(transfer.0.recipient().clone())
        .satoshis(BRC20_INSCRIBE_AMOUNT)
        .build()
        .unwrap();

    let output_change = TxOutputP2WPKH::builder()
        .recipient(alice.try_into().unwrap())
        .satoshis(FULL_AMOUNT - BRC20_INSCRIBE_AMOUNT - MINER_FEE)
        .build()
        .unwrap();

    let transaction = TransactionBuilder::new()
        .add_input(input.into())
        .add_output(output.into())
        .add_output(output_change.into())
        .sign_inputs(alice)
        .unwrap()
        .serialize()
        .unwrap();

	let hex = hex::encode(&transaction, false);
	assert_eq!(hex, "020000000001019ba48f8b1424c916e66927d0db48a1ea030e0974b287699a63657be322ea21110100000000ffffffff02581b000000000000225120e8b706a97732e705e22ae7710703e7f589ed13c636324461afa443016134cc050474000000000000160014e311b8d6ddff856ce8e9a4e03bc6d4fe5050a83d02483045022100ed8e58f97f39a4495b5e8701b3a0838b60b9d36ef74dd3feda044fb0c0337ac50220429f32cf166c3d7529c9627aff1c8ddb8802fb2cde8ad1afe61d8aef7b54b5ec0121030f209b6ada5edb42c77fd2bc64ad650ae38314c8f451f3e36d80bc8e26f132cb00000000");
	println!("{hex}");
	
	// Reveal transfer.
    let txid = Txid::from_str("df800a5234945b46574426ff1e99674deaea3749a50838542bd5f909c56ea93c").unwrap();

    let input = TxInputP2TRScriptPath::builder()
        .txid(txid)
        .vout(0)
        .recipient(transfer.0.recipient().clone())
        .satoshis(BRC20_INSCRIBE_AMOUNT)
		.script(transfer.0.envelope.script)
		.spend_info(transfer.0.envelope.spend_info)
        .build()
        .unwrap();

    let output = TxOutputP2WPKH::builder()
        .recipient(alice.try_into().unwrap())
        .satoshis(BRC20_DUST_AMOUNT)
        .build()
        .unwrap();

    let transaction = TransactionBuilder::new()
        .add_input(input.into())
        .add_output(output.into())
        .sign_inputs(alice)
        .unwrap()
        .serialize()
        .unwrap();

	const EXPECTED: &str = "020000000001013ca96ec509f9d52b543808a54937eaea4d67991eff264457465b9434520a80df0000000000ffffffff012202000000000000160014e311b8d6ddff856ce8e9a4e03bc6d4fe5050a83d0340eb824d08b9f2c167687c21d5a28705f5066452e19aba05747d80ae019c871d18c666dda1878d40c5e0664254f9dadc587c1538d118eca7a318fc1afbb047581c5b0063036f7264010118746578742f706c61696e3b636861727365743d7574662d3800377b2270223a226272632d3230222c226f70223a227472616e73666572222c227469636b223a226f616466222c22616d74223a223230227d6821c00f209b6ada5edb42c77fd2bc64ad650ae38314c8f451f3e36d80bc8e26f132cb00000000";
	let hex = hex::encode(&transaction, false);

	assert_eq!(hex[..164], EXPECTED[..164]);
	assert_eq!(hex[292..], EXPECTED[292..]);
	// We skip the 64-byte Schnorr signature, which uses random data each time
	// and is therefore not reproducible.
	assert_ne!(hex[164..292], EXPECTED[164..292]);

	println!("{hex}");
}
