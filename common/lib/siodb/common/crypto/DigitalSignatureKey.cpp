// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "DigitalSignatureKey.h"
#include "openssl_wrappers/BigNum.h"
#include "openssl_wrappers/BioMemBuf.h"
#include "openssl_wrappers/DsaKey.h"
#include "openssl_wrappers/EcKey.h"
#include "openssl_wrappers/EcPoint.h"
#include "openssl_wrappers/EvpKey.h"
#include "openssl_wrappers/EvpMdCtx.h"
#include "openssl_wrappers/EvpPkeyCtx.h"
#include "openssl_wrappers/RsaKey.h"

// STL headers
#include <algorithm>
#include <array>
#include <sstream>
#include <vector>

// Boost headers
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/endian/conversion.hpp>

// OpenSSL headers
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

namespace siodb::crypto {
namespace {

int errorPasswordCallback([[maybe_unused]] char* buf, [[maybe_unused]] int size,
        [[maybe_unused]] int rwflag, [[maybe_unused]] void* u)
{
    // -1 means error
    return -1;
}

enum class KeyType { kRsa, kDsa, kEcdsa, kEd25519, kUnknown };

constexpr std::size_t kEd25519strSize = 32;
constexpr std::size_t kEd25519PrivateKeySize = 64;
constexpr std::size_t kEd25519SignatureSize = 64;
constexpr int kMinRsaKeyLength = 2048;
constexpr auto kKeyTypesCount = static_cast<std::size_t>(KeyType::kUnknown);

const std::array<const char*, 1> kOpenSshRsaKeyNames = {"ssh-rsa"};
const std::array<const char*, 1> kOpenSshDsaKeyNames = {"ssh-dss"};
const std::array<const char*, 3> kOpenSshEcDsaKeyNames = {
        "ecdsa-sha2-nistp256", "ecdsa-sha2-nistp384", "ecdsa-sha2-nistp521"};
const std::array<const char*, 1> kOpenSshEd25519KeyNames = {"ssh-ed25519"};

const std::array<std::pair<const char*, int>, 3> kEcCurveNames = {
        std::make_pair<const char*, int>("nistp256", NID_X9_62_prime256v1),
        {"nistp384", NID_secp384r1}, {"nistp521", NID_secp521r1}};

/** OpenSSH formatted keys */
constexpr const char* kOpenSshAuthMagic = "openssh-key-v1";
constexpr const char* kOpenSshHeaderBegin = "-----BEGIN OPENSSH PRIVATE KEY-----";
constexpr const char* kOpenSshFooter = "-----END OPENSSH PRIVATE KEY-----";

constexpr const char* kNoneCipher = "none";

KeyType parseOpenSshType(utils::StringScanner& scanner) noexcept
{
    const auto rsaIt = std::find_if(
            kOpenSshRsaKeyNames.begin(),
            kOpenSshRsaKeyNames.end(), [&scanner](const auto& str) noexcept {
                return scanner.startsWith(str);
            });
    if (rsaIt != kOpenSshRsaKeyNames.end()) {
        scanner.advance(std::strlen(*rsaIt));
        return KeyType::kRsa;
    }

    const auto dsaIt = std::find_if(
            kOpenSshDsaKeyNames.begin(),
            kOpenSshDsaKeyNames.end(), [&scanner](const auto& str) noexcept {
                return scanner.startsWith(str);
            });
    if (dsaIt != kOpenSshDsaKeyNames.end()) {
        scanner.advance(std::strlen(*dsaIt));
        return KeyType::kDsa;
    }

    const auto ecDsaIt = std::find_if(
            kOpenSshEcDsaKeyNames.begin(),
            kOpenSshEcDsaKeyNames.end(), [&scanner](const auto& str) noexcept {
                return scanner.startsWith(str);
            });
    if (ecDsaIt != kOpenSshEcDsaKeyNames.end()) {
        scanner.advance(std::strlen(*ecDsaIt));
        return KeyType::kEcdsa;
    }

    const auto ed25519It = std::find_if(
            kOpenSshEd25519KeyNames.begin(),
            kOpenSshEd25519KeyNames.end(), [&scanner](const auto& str) noexcept {
                return scanner.startsWith(str);
            });
    if (ed25519It != kOpenSshEd25519KeyNames.end()) {
        scanner.advance(std::strlen(*ed25519It));
        return KeyType::kEd25519;
    }

    return KeyType::kUnknown;
}

std::uint32_t readOpenSshEncodedSize(utils::StringScanner& scanner)
{
    std::uint32_t length = 0;
    if (!scanner.read(&length, sizeof(length)))
        throw std::runtime_error("Read OpenSSH encoded size failed");
    boost::endian::big_to_native_inplace(length);
    return length;
}

BigNum readBigNum(utils::StringScanner& scanner)
{
    const std::uint32_t bigNumLength = readOpenSshEncodedSize(scanner);
    if (bigNumLength > scanner.remainingSize()) throw std::runtime_error("Read big number failed");

    BigNum bn(reinterpret_cast<const unsigned char*>(scanner.current()), bigNumLength);
    scanner.advance(bigNumLength);
    return bn;
}

void checkRsaLength(const RSA* rsa)
{
    const BIGNUM* rsaN;
    RSA_get0_key(rsa, &rsaN, nullptr, nullptr);
    const auto bnBits = BN_num_bits(rsaN);
    if (bnBits < kMinRsaKeyLength)
        throw std::runtime_error("RSA key has less than 2048 bits length");
}

/** 
 * Decodes Base64 Blob data without new lines
 */
std::string decodeBase64(const char* str, size_t size)
{
    using namespace boost::archive::iterators;
    using ItBinaryT = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

    std::string output(ItBinaryT(str), ItBinaryT(str + size));
    return output;
}

/** 
 * Decodes Base64 Blob data with new lines
 */
std::string decodeBase64WithNewLines(const char* str, size_t size)
{
    using namespace boost::archive::iterators;
    using ItBinaryT =
            transform_width<binary_from_base64<remove_whitespace<std::string::const_iterator>>, 8,
                    6>;

    std::string output(ItBinaryT(str), ItBinaryT(str + size));
    return output;
}

void readAndCheckKeyType(
        utils::StringScanner& scanner, const char* const* begin, const char* const* end)
{
    // Key type is encoded in key blob data
    const auto stringSize = readOpenSshEncodedSize(scanner);
    if (stringSize > scanner.remainingSize()) throw std::runtime_error("Invalid type string size");

    const auto it = std::find_if(
            begin, end, [&scanner](const auto& str) noexcept { return scanner.startsWith(str); });

    if (it == end) throw std::runtime_error("Unknown access key type");
    scanner.advance(stringSize);
}

int readAndCheckEcCurveType(utils::StringScanner& scanner)
{
    // Key type is encoded in key blob data
    const auto stringSize = readOpenSshEncodedSize(scanner);
    if (stringSize > scanner.remainingSize()) throw std::runtime_error("Invalid type string size");

    const auto it = std::find_if(
            kEcCurveNames.begin(),
            kEcCurveNames.end(), [&scanner](const auto& ecNameIdPair) noexcept {
                return scanner.startsWith(ecNameIdPair.first);
            });

    if (it == kEcCurveNames.end()) throw std::runtime_error("Unknown access key type");
    scanner.advance(stringSize);

    return it->second;
}

void readECPoint(utils::StringScanner& scanner, EC_POINT* v, const EC_GROUP* g)
{
    const auto length = readOpenSshEncodedSize(scanner);

    if (*scanner.current() != POINT_CONVERSION_UNCOMPRESSED)
        throw std::runtime_error("Only uncompressed points is allowed for EC");

    const auto dataPtr = reinterpret_cast<const unsigned char*>(scanner.current());
    if (EC_POINT_oct2point(g, v, dataPtr, length, nullptr) != 1)
        throw OpenSslError("EC_POINT_oct2point failed");
}

void validateEcPublicKey(const EC_GROUP* group, const EC_POINT* qPoint)
{
    BigNum order, x, y, tmp;
    if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) != NID_X9_62_prime_field)
        throw std::runtime_error("EC_METHOD is invalid");

    /* Q != infinity */
    if (EC_POINT_is_at_infinity(group, qPoint) == 1)
        throw std::runtime_error("EC_METHOD: EC_POINT is unreachable");

    if (EC_GROUP_get_order(group, order, nullptr) != 1
            || EC_POINT_get_affine_coordinates_GFp(group, qPoint, x, y, nullptr) != 1)
        throw std::runtime_error("EC_GROUP values is invalid");

    if (BN_num_bits(x) <= BN_num_bits(order) / 2 || BN_num_bits(y) <= BN_num_bits(order) / 2)
        throw std::runtime_error("EC_GROUP values is invalid");

    EcPoint nq(group);
    if (EC_POINT_mul(group, nq, nullptr, qPoint, order, nullptr) != 1)
        throw OpenSslError("EC_POINT_mul failed");

    if (EC_POINT_is_at_infinity(group, nq) != 1)
        throw std::runtime_error("EC_POINT should be infinity");

    if (!BN_sub(tmp, order, BN_value_one())) throw std::runtime_error("EC_GROUP is invalid");

    if (BN_cmp(x, tmp) >= 0 || BN_cmp(y, tmp) >= 0)
        throw std::runtime_error("EC_POINT should be infinity");
}

}  // namespace

void DigitalSignatureKey::parseOpenSslKey(utils::StringScanner& scanner)
{
    // Just read a key with OpenSSL. OpenSSL could parse PEM and PKCS8 keys.
    const bool privateKey = scanner.findInLine("PRIVATE") != std::string::npos;
    BioMemBuf keyBio(scanner.data(), scanner.size());
    EvpKeyPtr key;

    // TODO: Support password protected keys
    if (privateKey) {
        key = std::make_shared<EvpKey>(
                PEM_read_bio_PrivateKey(keyBio, nullptr, errorPasswordCallback, nullptr));
    } else {
        key = std::make_shared<EvpKey>(
                PEM_read_bio_PUBKEY(keyBio, nullptr, errorPasswordCallback, nullptr));
    }

    if (!key) throw OpenSslError("SSL key parse failed");

    if (EVP_PKEY_base_id(*key) == EVP_PKEY_RSA) {
        const auto rsa = EVP_PKEY_get0_RSA(*key);
        if (rsa == nullptr) throw OpenSslError("Invalid RSA key");

        checkRsaLength(rsa);
    }

    m_key = std::move(key);
}

void DigitalSignatureKey::parseOpenSshRsaPublicKey(const std::string& str)
{
    utils::StringScanner scanner(str);
    readAndCheckKeyType(scanner, kOpenSshRsaKeyNames.begin(), kOpenSshRsaKeyNames.end());

    RsaKey rsa;
    BigNum rsaE = std::move(readBigNum(scanner));
    BigNum rsaN = std::move(readBigNum(scanner));
    if (RSA_set0_key(rsa, rsaN, rsaE, nullptr) != 1) throw OpenSslError("RSA_set0_key failed");

    // Values are in key now
    rsaE.release();
    rsaN.release();

    checkRsaLength(rsa);
    auto evpKey = std::make_shared<EvpKey>();
    if (EVP_PKEY_set1_RSA(*evpKey, rsa) == 0) throw OpenSslError("EVP_PKEY_set1_RSA failed");

    rsa.release();
    m_key = std::move(evpKey);
}

void DigitalSignatureKey::parseOpenSshDsaPublicKey(const std::string& str)
{
    utils::StringScanner scanner(str);
    readAndCheckKeyType(scanner, kOpenSshDsaKeyNames.begin(), kOpenSshDsaKeyNames.end());

    DsaKey dsa;

    auto dsaP = readBigNum(scanner);
    auto dsaQ = readBigNum(scanner);
    auto dsaG = readBigNum(scanner);
    if (DSA_set0_pqg(dsa, dsaP, dsaQ, dsaG) == 0) throw OpenSslError("DSA_set0_pqg failed");

    // Values are in key now
    dsaP.release();
    dsaQ.release();
    dsaG.release();

    auto dsaPubKey = readBigNum(scanner);
    if (DSA_set0_key(dsa, dsaPubKey, nullptr) == 0) throw OpenSslError("DSA_set0_key failed");

    dsaPubKey.release();

    auto evpKey = std::make_shared<EvpKey>();
    if (EVP_PKEY_set1_DSA(*evpKey, dsa) == 0) throw OpenSslError("EVP_PKEY_set1_DSA failed");

    dsa.release();
    m_key = std::move(evpKey);
}

void DigitalSignatureKey::parseOpenSshEcdsaPublicKey(const std::string& str)
{
    utils::StringScanner scanner(str);
    readAndCheckKeyType(scanner, kOpenSshEcDsaKeyNames.begin(), kOpenSshEcDsaKeyNames.end());

    const auto curveId = readAndCheckEcCurveType(scanner);
    EcKey ecKey(curveId);
    EcPoint qPoint(EC_KEY_get0_group(ecKey));

    readECPoint(scanner, qPoint, EC_KEY_get0_group(ecKey));
    validateEcPublicKey(EC_KEY_get0_group(ecKey), qPoint);
    if (EC_KEY_set_public_key(ecKey, qPoint) != 1)
        throw OpenSslError("EC_KEY_set_public_key failed");

    // Point is in key now
    qPoint.release();

    auto evpKey = std::make_shared<EvpKey>();
    if (EVP_PKEY_set1_EC_KEY(*evpKey, ecKey) == 0)
        throw OpenSslError("EVP_PKEY_set1_EC_KEY failed");

    ecKey.release();
    m_key = std::move(evpKey);
}

void DigitalSignatureKey::parseOpenSshEd25519PublicKey(const std::string& str)
{
    utils::StringScanner scanner(str);
    readAndCheckKeyType(scanner, kOpenSshEd25519KeyNames.begin(), kOpenSshEd25519KeyNames.end());
    const auto publicKeyLength = readOpenSshEncodedSize(scanner);
    if (publicKeyLength != kEd25519strSize)
        throw std::runtime_error("ED25519 length is not equal to 32 bytes");

    auto evpKey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL,
            reinterpret_cast<const unsigned char*>(scanner.current()), kEd25519strSize);

    if (!evpKey) throw OpenSslError("EVP_PKEY_new failed");

    m_key = std::make_shared<EvpKey>(evpKey);
}

void DigitalSignatureKey::parseOpenSshPrivateKey(const std::string& str)
{
    utils::StringScanner scanner(str);
    if (!scanner.startsWith(kOpenSshAuthMagic))
        throw std::runtime_error("OpenSSH private key is invalid");

    scanner.advance(std::strlen(kOpenSshAuthMagic) + 1);  // + skip '0' char

    // Read Cipher name
    auto length = readOpenSshEncodedSize(scanner);
    if (length > scanner.remainingSize())
        throw std::runtime_error("OpenSSH private key is invalid");

    // TODO: support encrypted keys
    if (!scanner.startsWith(kNoneCipher))
        throw std::runtime_error("Encrypted keys are not supported");

    scanner.advance(length);

    // Read 'KDF' name
    length = readOpenSshEncodedSize(scanner);
    if (length > scanner.remainingSize())
        throw std::runtime_error("OpenSSH private key is invalid");

    scanner.advance(length);

    // Read 'KDF' data
    length = readOpenSshEncodedSize(scanner);
    if (length > scanner.remainingSize())
        throw std::runtime_error("OpenSSH private key is invalid");

    scanner.advance(length);

    const auto numKeys = readOpenSshEncodedSize(scanner);
    if (numKeys != 1) throw std::runtime_error("Only single key per file is allowed");

    // Read unencrypted public key size.
    length = readOpenSshEncodedSize(scanner);
    if (length > scanner.remainingSize())
        throw std::runtime_error("OpenSSH private key is invalid");
    // Skip public key
    scanner.advance(length);
    // Read private key size.
    const auto privateKeySize = readOpenSshEncodedSize(scanner);
    if (privateKeySize > scanner.remainingSize())
        throw std::runtime_error("OpenSSH private key is invalid");

    // Skip 2 check numbers.
    readOpenSshEncodedSize(scanner);
    readOpenSshEncodedSize(scanner);

    const auto keyFormatSize = readOpenSshEncodedSize(scanner);
    if (keyFormatSize > scanner.remainingSize())
        throw std::runtime_error("Invalid OpenSSH key type string size");

    if (std::find_if(
                kOpenSshEd25519KeyNames.begin(), kOpenSshEd25519KeyNames.end(),
                [&scanner](const auto& str) noexcept { return scanner.startsWith(str); })
            != kOpenSshEd25519KeyNames.end()) {
        scanner.advance(keyFormatSize);
        parseOpenSshEd25519PrivateKey(scanner);
    } else if (std::find_if(
                       kOpenSshRsaKeyNames.begin(), kOpenSshRsaKeyNames.end(),
                       [&scanner](const auto& str) noexcept { return scanner.startsWith(str); })
               != kOpenSshRsaKeyNames.end()) {
        // TODO: Support OpenSSH RSA key type
        throw std::runtime_error("OpenSSH RSA private key format is unsupported");
    } else if (std::find_if(
                       kOpenSshDsaKeyNames.begin(), kOpenSshDsaKeyNames.end(),
                       [&scanner](const auto& str) noexcept { return scanner.startsWith(str); })
               != kOpenSshDsaKeyNames.end()) {
        // TODO: Support OpenSSH DSA key type
        throw std::runtime_error("OpenSSH DSA private key format is unsupported");
    } else if (std::find_if(
                       kOpenSshEcDsaKeyNames.begin(), kOpenSshEcDsaKeyNames.end(),
                       [&scanner](const auto& str) noexcept { return scanner.startsWith(str); })
               != kOpenSshEcDsaKeyNames.end()) {
        // TODO: Support OpenSSH ECDSA key type
        throw std::runtime_error("OpenSSH ECDSA private key format is unsupported");
    } else {
        throw std::runtime_error("Unsupported OpenSSH private key format");
    }
}

void DigitalSignatureKey::parseOpenSshEd25519PrivateKey(utils::StringScanner& scanner)
{
    // Skip public key raw data
    auto length = readOpenSshEncodedSize(scanner);
    scanner.advance(length);
    // Read private key raw data size
    length = readOpenSshEncodedSize(scanner);

    // Private key consist of private and public raw key string
    if (length != kEd25519PrivateKeySize)
        throw std::runtime_error("ED25519 private key size should be 64 (priv + pub)");

    // Read only private part from private key
    auto evpKey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL,
            reinterpret_cast<const unsigned char*>(scanner.current()), kEd25519strSize);

    if (!evpKey) throw OpenSslError("EVP_PKEY_new_raw_private_key failed");

    m_key = std::make_shared<EvpKey>(evpKey);
}

void DigitalSignatureKey::parseOpenSshPublicKey(utils::StringScanner& scanner)
{
    m_key.reset();
    if (!scanner.skipWhitespaces()) throw std::runtime_error("Invalid OpenSSH public key string");

    const auto type = parseOpenSshType(scanner);
    if (type == KeyType::kUnknown) throw std::runtime_error("Unknown RFC4716 key type");

    scanner.skipWhitespaces();
    if (!scanner.hasMoreData()) throw std::runtime_error("Invalid OpenSSH public key string");

    const auto dataBlobStart = scanner.pos();
    // Read until key string end
    scanner.skipUntilWhitespace();
    const auto decodedstr =
            decodeBase64(scanner.data() + dataBlobStart, scanner.pos() - dataBlobStart);

    switch (type) {
        case KeyType::kRsa: parseOpenSshRsaPublicKey(decodedstr); break;
        case KeyType::kDsa: parseOpenSshDsaPublicKey(decodedstr); break;
        case KeyType::kEcdsa: parseOpenSshEcdsaPublicKey(decodedstr); break;
        case KeyType::kEd25519: parseOpenSshEd25519PublicKey(decodedstr); break;
        default: throw std::runtime_error("Invalid OpenSSH key type");
    }
}

void DigitalSignatureKey::parseFromString(const char* str, std::size_t size)
{
    utils::StringScanner scanner(str, size);

    // First check OpenSSH key format
    if (scanner.startsWith(kOpenSshHeaderBegin)) {
        // OpenSSH key is not supported format by OpenSSL, parse key manualy
        scanner.advance(std::strlen(kOpenSshHeaderBegin));
        if (!scanner.skipWhitespaces()) throw std::runtime_error("Invalid OpenSSH key string");

        auto dataBlobSize = scanner.find(kOpenSshFooter);
        if (dataBlobSize == std::string::npos)
            throw std::runtime_error("OpenSSH footer is not found");

        // -1 to not include '-' from footer
        --dataBlobSize;
        const auto decodedstr = decodeBase64WithNewLines(scanner.current(), dataBlobSize);
        parseOpenSshPrivateKey(decodedstr);
    } else if (scanner.startsWith("-----"))
        parseOpenSslKey(scanner);
    else
        parseOpenSshPublicKey(scanner);
}

std::string DigitalSignatureKey::createMessageDigest(const std::string& msg) const
{
    EvpMdCtx ctx;
    const EVP_MD* md = EVP_sha512();
    if (md == nullptr) throw OpenSslError("EVP_sha512 failed");

    if (EVP_DigestInit(ctx, md) != 1) throw OpenSslError("EVP_DigestInit failed");

    std::string digest(EVP_MD_size(md), '\0');
    if (EVP_DigestUpdate(ctx, msg.data(), msg.size()) != 1)
        throw OpenSslError("EVP_DigestInit failed");

    if (EVP_DigestFinal(ctx, reinterpret_cast<unsigned char*>(digest.data()), nullptr) != 1)
        throw OpenSslError("EVP_DigestInit failed");

    return digest;
}

std::string DigitalSignatureKey::signMessage(const std::string& msg) const
{
    // ED25519 supports only one-shot digest signing and verifying
    if (EVP_PKEY_base_id(*m_key) == EVP_PKEY_ED25519) return signMessageEd25519(msg);

    EvpPKeyCtx ctx(*m_key, nullptr);

    if (EVP_PKEY_sign_init(ctx) <= 0) throw OpenSslError("EVP_PKEY_sign_init failed");

    if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha512()) <= 0)
        throw OpenSslError("EVP_PKEY_CTX_set_signature_md failed");

    const auto digest = createMessageDigest(msg);
    std::size_t signatureSize = 0;

    /* Determine buffer length */
    if (EVP_PKEY_sign(ctx, nullptr, &signatureSize,
                reinterpret_cast<const unsigned char*>(digest.data()), digest.size())
            <= 0)
        throw OpenSslError("EVP_PKEY_sign failed");

    // Now signatureSize contains maximum size;
    std::string signature(signatureSize, '\0');
    if (EVP_PKEY_sign(ctx, reinterpret_cast<unsigned char*>(signature.data()), &signatureSize,
                reinterpret_cast<const unsigned char*>(digest.data()), digest.size())
            <= 0)
        throw OpenSslError("EVP_PKEY_sign failed");

    // signatureSize contains real size now. Update signature size
    signature.resize(signatureSize);

    return signature;
}

std::string DigitalSignatureKey::signMessageEd25519(const std::string& msg) const
{
    std::string signature;
    EvpMdCtx ctx;
    signature.resize(kEd25519SignatureSize, '\0');
    std::size_t signatureSize = kEd25519SignatureSize;
    EVP_DigestSignInit(ctx, NULL, NULL, NULL, *m_key);

    const int result = EVP_DigestSign(ctx, reinterpret_cast<unsigned char*>(signature.data()),
            &signatureSize, reinterpret_cast<const unsigned char*>(msg.data()), msg.size());

    if (result <= 0) throw OpenSslError("EVP_DigestSign for Ed25519 failed");

    return signature;
}

bool DigitalSignatureKey::verifySignature(
        const std::string& message, const std::string& signature) const
{
    // ED25519 supports only one-shot digest signing and verifying
    const auto keyBaseId = EVP_PKEY_base_id(*m_key);
    if (keyBaseId == EVP_PKEY_ED25519) return verifySignatureEd25519(message, signature);

    EvpPKeyCtx ctx(*m_key, nullptr);
    if (EVP_PKEY_verify_init(ctx) <= 0) throw OpenSslError("EVP_PKEY_verify_init failed");

    if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha512()) <= 0)
        throw OpenSslError("EVP_PKEY_CTX_set_signature_md failed");

    const auto digest = createMessageDigest(message);
    const auto rc = EVP_PKEY_verify(ctx, reinterpret_cast<const unsigned char*>(signature.data()),
            signature.size(), reinterpret_cast<const unsigned char*>(digest.data()), digest.size());

    return rc == 1;
}

bool DigitalSignatureKey::verifySignatureEd25519(
        const std::string& message, const std::string& signature) const
{
    EvpMdCtx ctx;
    const auto rc = EVP_DigestVerifyInit(ctx, NULL, NULL, NULL, *m_key);
    if (rc <= 0) throw OpenSslError("EVP_DigestVerifyInit failed");

    const bool result = EVP_DigestVerify(ctx,
            reinterpret_cast<const unsigned char*>(signature.data()), signature.size(),
            reinterpret_cast<const unsigned char*>(message.data()), message.size());

    return result;
}

}  // namespace siodb::crypto
