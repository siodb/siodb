// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AesCipher.h"

// Project headers
#include "AesCipherContext.h"

namespace siodb::iomgr::dbengine::crypto {

///// class AesCipher //////////////////////////////////////////////////////////

unsigned AesCipher::getBlockSize() const noexcept
{
    return 128;
}

// ---- internals ----

CipherContextPtr AesCipher::doCreateEncryptionContext(const BinaryValue& key) const
{
    return std::make_shared<AesEncryptionContext>(shared_from_this(), key);
}

CipherContextPtr AesCipher::doCreateDecryptionContext(const BinaryValue& key) const
{
    return std::make_shared<AesDecryptionContext>(shared_from_this(), key);
}

///// class Aes128 /////////////////////////////////////////////////////////////

const char* Aes128::getCipherId() const noexcept
{
    return "aes128";
}

unsigned Aes128::getKeySize() const noexcept
{
    return 128;
}

///// class Aes192 /////////////////////////////////////////////////////////////

const char* Aes192::getCipherId() const noexcept
{
    return "aes192";
}

unsigned Aes192::getKeySize() const noexcept
{
    return 192;
}

///// class Aes256 /////////////////////////////////////////////////////////////

const char* Aes256::getCipherId() const noexcept
{
    return "aes256";
}

unsigned Aes256::getKeySize() const noexcept
{
    return 256;
}

}  // namespace siodb::iomgr::dbengine::crypto
