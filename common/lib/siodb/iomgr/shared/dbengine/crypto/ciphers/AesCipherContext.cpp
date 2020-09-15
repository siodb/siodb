// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AesCipherContext.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

namespace siodb::iomgr::dbengine::crypto {

////////// class AesEncryptionContext ////////////////////////////////////

void AesEncryptionContext::transform(
        const void* in, std::size_t blockCount, void* out) const noexcept
{
    for (; blockCount > 0; --blockCount,
            in = static_cast<const std::uint8_t*>(in) + m_blockSizeInBytes,
            out = static_cast<std::uint8_t*>(out) + m_blockSizeInBytes) {
        AES_encrypt(static_cast<const unsigned char*>(in), static_cast<unsigned char*>(out),
                &m_preparedKey);
    }
}

////////// class AesDecryptionContext ////////////////////////////////////

void AesDecryptionContext::transform(
        const void* in, std::size_t blockCount, void* out) const noexcept
{
    for (; blockCount > 0; --blockCount,
            in = static_cast<const std::uint8_t*>(in) + m_blockSizeInBytes,
            out = static_cast<std::uint8_t*>(out) + m_blockSizeInBytes) {
        AES_decrypt(static_cast<const unsigned char*>(in), static_cast<unsigned char*>(out),
                &m_preparedKey);
    }
}

}  // namespace siodb::iomgr::dbengine::crypto
