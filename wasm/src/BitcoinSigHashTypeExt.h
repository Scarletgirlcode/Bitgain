#pragma once

#include <TrustWalletCore/TWBitcoinSigHashType.h>

#include <emscripten/bind.h>

using namespace emscripten;

namespace TW::Wasm {

    class BitcoinSigHashTypeExt {
    public:
        static auto isSingle(TWBitcoinSigHashType type);
        static auto isNone(TWBitcoinSigHashType type);
    };
}
