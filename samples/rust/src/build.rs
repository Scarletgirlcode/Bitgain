static WALLET_CORE_PROJECT_DIR: &str = "../../build";
    // libs to link with
    static LIBS: [&str; 3] = ["TrustWalletCore", "TrezorCrypto", "protobuf"];

fn main() {
    println!("cargo:rustc-link-search=native={}", WALLET_CORE_PROJECT_DIR);
    println!("cargo:rustc-link-search=native={}/trezor-crypto", WALLET_CORE_PROJECT_DIR);

    for i in 0..LIBS.len() {
        println!("cargo:rustc-link-lib={}", LIBS[i]);
    }
    if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=c++");
    } else { // "linux", etc
        println!("cargo:rustc-link-lib=stdc++");
    }
}
