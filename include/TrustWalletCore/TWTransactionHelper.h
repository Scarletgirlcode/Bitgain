// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "TWBase.h"
#include "TWCoinType.h"
#include "TWData.h"
#include "TWString.h"

TW_EXTERN_C_BEGIN

/// Non-core transaction utility methods
TW_EXPORT_CLASS
struct TWTransactionHelper;

/// Obtain pre-signing hash of a transaction.
TW_EXPORT_STATIC_METHOD
TWData *_Nonnull TWTransactionHelperPreImageHash(TWCoinType coinType, TWData *_Nonnull txInputData);

/// Compile a complete transation with provided signature, put together from transaction input and provided public key and signature
TW_EXPORT_STATIC_METHOD
TWData *_Nonnull TWTransactionHelperCompileWithSignature(TWCoinType coinType, TWData *_Nonnull txInputData, TWData *_Nonnull signature, TWData *_Nonnull publicKey);

TW_EXTERN_C_END
