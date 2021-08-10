// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Cipher.h"

// Project headers
#include "AesCipher.h"
#include "CamelliaCipher.h"

// STL headers
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace siodb::iomgr::dbengine::crypto {

namespace {

std::unordered_map<std::string_view, CipherPtr> g_ciphers;

void addCipher(const CipherPtr& cipher)
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

// --- internals ---

void Cipher::validateKeyLength(const BinaryValue& key) const
{
    const auto expectedKeySize = getKeySizeInBits() / 8;
    if (key.size() != expectedKeySize) {
        std::ostringstream err;
        err << "Invalid cipher key size for the cipher " << getCipherId() << ": expecting "
            << expectedKeySize << " bytes, but received " << key.size() << " bytes";
        throw std::runtime_error(err.str());
    }
    //        throwDatabaseError(IOManagerMessageId::kErrorInvalidCipherKey, getCipherId());
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

std::optional<CipherPtr> getCipher0(const std::string& cipherId)
{
    if (cipherId == kNoCipherId) return nullptr;
    const auto it = g_ciphers.find(std::string_view(cipherId));
    if (it != g_ciphers.end()) return it->second;
    return std::nullopt;
}

}  // namespace siodb::iomgr::dbengine::crypto
