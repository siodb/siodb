// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CamelliaCipher.h"

// Project headers
#include "CamelliaCipherContext.h"

namespace siodb::iomgr::dbengine::crypto {

///// class CamelliaCipher /////////////////////////////////////////////////////

unsigned CamelliaCipher::getBlockSize() const noexcept
{
    return 128;
}

// ----- internals -----

CipherContextPtr CamelliaCipher::doCreateEncryptionContext(const BinaryValue& key) const
{
    return std::make_shared<CamelliaEncryptionContext>(shared_from_this(), key);
}

CipherContextPtr CamelliaCipher::doCreateDecryptionContext(const BinaryValue& key) const
{
    return std::make_shared<CamelliaDecryptionContext>(shared_from_this(), key);
}

///// class Camellia128 ////////////////////////////////////////////////////////

const char* Camellia128::getCipherId() const noexcept
{
    return "camellia128";
}

unsigned Camellia128::getKeySize() const noexcept
{
    return 128;
}

///// class Camellia192 ////////////////////////////////////////////////////////

const char* Camellia192::getCipherId() const noexcept
{
    return "camellia192";
}

unsigned Camellia192::getKeySize() const noexcept
{
    return 192;
}

///// class Camellia256 ////////////////////////////////////////////////////////

const char* Camellia256::getCipherId() const noexcept
{
    return "camellia256";
}

unsigned Camellia256::getKeySize() const noexcept
{
    return 256;
}

}  // namespace siodb::iomgr::dbengine::crypto
