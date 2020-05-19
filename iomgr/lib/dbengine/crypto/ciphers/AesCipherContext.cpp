// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AesCipherContext.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

namespace siodb::iomgr::dbengine::crypto {

////////// class AesEncryptionContext ////////////////////////////////////

void AesEncryptionContext::transform(
        const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const noexcept
{
    for (; blockCount > 0; --blockCount, in += m_blockSizeInBytes, out += m_blockSizeInBytes)
        AES_encrypt(in, out, &m_preparedKey);
}

////////// class AesDecryptionContext ////////////////////////////////////

void AesDecryptionContext::transform(
        const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const noexcept
{
    for (; blockCount > 0; --blockCount, in += m_blockSizeInBytes, out += m_blockSizeInBytes)
        AES_decrypt(in, out, &m_preparedKey);
}

}  // namespace siodb::iomgr::dbengine::crypto
