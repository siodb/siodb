// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "GetCipher.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/Cipher.h>

namespace siodb::iomgr::dbengine::crypto {

CipherPtr getCipher(const std::string& cipherId)
{
    auto cipher = getCipher0(cipherId);
    if (cipher) return *cipher;
    throwDatabaseError(IOManagerMessageId::kErrorCipherUnknown, cipherId);
}

}  // namespace siodb::iomgr::dbengine::crypto
