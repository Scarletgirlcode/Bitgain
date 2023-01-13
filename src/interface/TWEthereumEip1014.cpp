// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Data.h"
#include "Ethereum/EIP1014.h"
#include <TrustWalletCore/TWEthereumEip1014.h>

#include <string>

TWString* TWEthereumEip1014AddressCreate2(TWString* _Nonnull fromEthAddress, TWData* _Nonnull salt, TWData* _Nonnull initCodeHash) {
    const auto& ethAddressStr = *reinterpret_cast<const std::string*>(fromEthAddress);
    const auto& saltData = *reinterpret_cast<const TW::Data*>(salt);
    const auto& initCodeHashData = *reinterpret_cast<const TW::Data*>(initCodeHash);
    return new std::string(TW::Ethereum::create2AddressString(ethAddressStr, saltData, initCodeHashData));
}
