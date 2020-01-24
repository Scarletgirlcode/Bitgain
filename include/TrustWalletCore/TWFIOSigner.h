// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "TWBase.h"

#include "TWFIOProto.h"
#include "TWString.h"

TW_EXTERN_C_BEGIN

/// Represents a FIO Signer.
TW_EXPORT_CLASS
struct TWFIOSigner;

/// Build and sign a FIO message
TW_EXPORT_STATIC_METHOD
TW_FIO_Proto_SigningOutput TWFIOSignerSign(TW_FIO_Proto_SigningInput input);

/// Get default TPID
TW_EXPORT_STATIC_METHOD
TWString *_Nonnull TWFIOSignerGetDefaultTpid();

TW_EXTERN_C_END
