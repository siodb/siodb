// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Cipher.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "AesCipher.h"
#include "CamelliaCipher.h"
#include "../../ThrowDatabaseError.h"

// STL headers
#include <string_view>
#include <unordered_map>

namespace siodb::iomgr::dbengine::crypto {

namespace {

std::unordered_map<std::string_view, CipherPtr> g_ciphers;

inline void addCipher(const CipherPtr& cipher)
{
    g_ciphers.emplace(std::string_view(cipher->getCipherId()), cipher);
}

}  // anonymous namespace

////////// class Cipher ////////////////////////////////////////////////////////

CipherContextPtr Cipher::createEncryptionContext(const BinaryValue& key) const
{
    validateKeyLength(key);
    return doCreateEncryptionContext(key);
}

CipherContextPtr Cipher::createDecryptionContext(const BinaryValue& key) const
{
    validateKeyLength(key);
    return doCreateDecryptionContext(key);
}

// ----- internal -----

void Cipher::validateKeyLength(const BinaryValue& key) const
{
    if (key.size() != getKeySize() / 8)
        throwDatabaseError(IOManagerMessageId::kErrorInvalidCipherKey, getCipherId());
}

////////// STANDALONE FUNCTIONS ////////////////////////////////////////////////

void initializeBuiltInCiphers()
{
    addCipher(std::make_shared<Aes128>());
    addCipher(std::make_shared<Aes192>());
    addCipher(std::make_shared<Aes256>());
    addCipher(std::make_shared<Camellia128>());
    addCipher(std::make_shared<Camellia192>());
    addCipher(std::make_shared<Camellia256>());
}

void initializeExternalCiphers(
        [[maybe_unused]] const config::ExternalCipherOptions& externaCipherOptions)
{
    // TODO: Implement later, see SIODB-163
}

CipherPtr getCipher(const std::string& cipherId)
{
    if (cipherId == kNoCipherId) return nullptr;
    const auto it = g_ciphers.find(std::string_view(cipherId));
    if (it != g_ciphers.end()) return it->second;
    throwDatabaseError(IOManagerMessageId::kErrorCipherUnknown, cipherId);
}

}  // namespace siodb::iomgr::dbengine::crypto
