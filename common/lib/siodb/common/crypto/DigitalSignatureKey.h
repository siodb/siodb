// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OpenSslError.h"
#include "openssl_wrappers/EvpKey.h"
#include "../utils/StringScanner.h"

// STL headers
#include <memory>

// OpenSSL headers
#include <openssl/evp.h>

namespace siodb::crypto {

using EvpKeyPtr = std::shared_ptr<EvpKey>;

/** Cryptographic key for authentication*/
class DigitalSignatureKey {
public:
    /**
     * Parses key from a string.
     * @param str Key string.
     * @param size Key string size.
     * @throw OpenSslError in case of OpenSSL error.
     * @throw runtime_error in case of parsing error.
     */
    void parseFromString(const char* str, std::size_t size);

    /**
     * Parses key from a string.
     * @param str Key string.
     * @throw OpenSslError in case of OpenSSL error.
     * @throw runtime_error in case of parsing error.
     */
    void parseFromString(const std::string& str)
    {
        parseFromString(str.c_str(), str.size());
    }

    /**
     * Returns OpenSSL key.
     * @return OpenSSL Key string.
     */
    const EvpKeyPtr& getKey() const noexcept
    {
        return m_key;
    }

    /**
     * Signs message with a key.
     * @param msg Message.
     * @return Message signature.
     * @throw OpenSslError if OpenSSL error happens.
     */
    std::string signMessage(const std::string& msg) const;

    /**
     * Verifies message signature with a key.
     * @param msg Original message.
     * @param signature Signature.
     * @return true if signature is correct, false otherwise.
     * @throw OpenSslError if OpenSSL error happens.
     */
    bool verifySignature(const std::string& message, const std::string& signature) const;

private:
    /**
     * Parses public RSA key from decoded keyblob.
     * @param str Key blob data.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshRsaPublicKey(const std::string& str);

    /**
     * Parses public DSA key from decoded keyblob.
     * @param str Key blob data.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshDsaPublicKey(const std::string& str);

    /**
     * Parses public ECDSA key from decoded keyblob.
     * @param str Key blob data.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshEcdsaPublicKey(const std::string& str);

    /**
     * Parses public ED25519 key from decoded keyblob.
     * @param str Key blob data.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshEd25519PublicKey(const std::string& str);

    /**
     * Parses private OpenSSH key from decoded keyblob.
     * @param str Key blob data.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshPrivateKey(const std::string& str);

    /**
     * Parses private OpenSSH ED25519 key.
     * @param scanner Scanner.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshEd25519PrivateKey(utils::StringScanner& scanner);

    /**
     * Parses OpenSSL keys formats(PEM, PKCS8).
     * @param scanner String data scanner.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSslKey(utils::StringScanner& scanner);

    /**
     * Parses OpenSSL keys formats(PEM, PKCS8).
     * @param scanner String data scanner.
     * @throw runtime_error in case of parsing error.
     * @throw OpenSslError if OpenSSL error happens.
     */
    void parseOpenSshPublicKey(utils::StringScanner& scanner);

    /**
     * Creates message digest.
     * @param msg Message.
     * @return Message digest.
     * @throw OpenSslError if OpenSSL error happens.
     */
    std::string createMessageDigest(const std::string& msg) const;

    /**
     * Signs message with ED25519 key.
     * @param msg Message.
     * @return Message signature.
     * @throw OpenSslError if OpenSSL error happens.
     */
    std::string signMessageEd25519(const std::string& msg) const;

    /**
     * Verifies message signature with ED25519 key.
     * @param msg Original message.
     * @param signature Signature.
     * @return true if signature is correct, false otherwise.
     * @throw OpenSslError if OpenSSL error happens.
     */
    bool verifySignatureEd25519(const std::string& message, const std::string& signature) const;

private:
    /** OpenSSL key */
    EvpKeyPtr m_key;
};

}  // namespace siodb::crypto
