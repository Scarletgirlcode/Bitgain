# Steps to update TrezorCrypto lib to a newer version:
# - start with a clean wallet-core repo
# - clone fresh version of https://github.com/trezor/trezor-firmware
# - Copy contents of trezor-firmware/crypto to trezor-crypto/crypto
# - Run this script (or below commands) from trezor-crypto folder, to move over header files to include folder
# - Try compiling TrezorCrypto lib, wallet-core lib, TrezorCrypto tests, wallet-core tests
# - Execute tests
# - Do fixes as needed
# - Review changes

TARGET=include/TrezorCrypto

mv crypto/*.h $TARGET
mv crypto/aes/aes.h $TARGET
mv crypto/aes/*.h $TARGET/aes
mv crypto/ed25519-donna/ed25519.h $TARGET
mv crypto/ed25519-donna/ed25519-*.h $TARGET/ed25519-donna
mv crypto/chacha20poly1305/*.h $TARGET/chacha20poly1305
