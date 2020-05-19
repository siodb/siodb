// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CamelliaCipherContext.h"

namespace siodb::iomgr::dbengine::crypto {

////////// class CamelliaEncryptionContext ////////////////////////////////////

void CamelliaEncryptionContext::transform(
        const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const noexcept
{
    for (; blockCount > 0; --blockCount, in += m_blockSizeInBytes, out += m_blockSizeInBytes)
        Camellia_encrypt(in, out, &m_preparedKey);
}

////////// class CamelliaDecryptionContext ////////////////////////////////////

void CamelliaDecryptionContext::transform(
        const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const noexcept
{
    for (; blockCount > 0; --blockCount, in += m_blockSizeInBytes, out += m_blockSizeInBytes)
        Camellia_decrypt(in, out, &m_preparedKey);
}

}  // namespace siodb::iomgr::dbengine::crypto
