// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/crypto/DigitalSignatureKey.h>
#include <siodb/common/crypto/RandomGenerator.h>

// Google Test
#include <gtest/gtest.h>

namespace {

std::string createRandomMessage()
{
    siodb::crypto::RandomGenerator generator;
    std::string msg(1024, '\0');
    generator.getRandomBytes(reinterpret_cast<unsigned char*>(msg.data()), msg.size());
    return msg;
}

}  // namespace

void checkKey(const std::string& publicKeyStr, const std::string& privateKeyStr)
{
    const auto msg = createRandomMessage();
    std::string signature;
    {
        siodb::crypto::DigitalSignatureKey privateKey;
        privateKey.parseFromString(privateKeyStr);
        signature = privateKey.signMessage(msg);
    }

    {
        siodb::crypto::DigitalSignatureKey publicKey;
        publicKey.parseFromString(publicKeyStr);
        EXPECT_TRUE(publicKey.verifySignature(msg, signature));
        auto otherMsg = createRandomMessage();
        otherMsg[0] = msg[0] + 1;  // Make them never equal
        EXPECT_FALSE(publicKey.verifySignature(otherMsg, signature));
        ++signature[10];

        EXPECT_FALSE(publicKey.verifySignature(msg, signature));
    }

    try {
        auto invalidPubKey = publicKeyStr;
        ++invalidPubKey[1];
        {
            siodb::crypto::DigitalSignatureKey publicKey;
            publicKey.parseFromString(invalidPubKey);
            EXPECT_FALSE(publicKey.verifySignature(msg, signature));
            signature[10] = 'Z';
            EXPECT_FALSE(publicKey.verifySignature(msg, signature));
        }
    } catch (std::exception& e) {
        // Exception is possible but not mandatory
    }
}

TEST(DigitalSignatureKeyTest, RSA2048UnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ssh-rsa "
            "AAAAB3NzaC1yc2EAAAADAQABAAABAQDoBVv3EJHcAasNU4nYdJtdfCVeSH4+"
            "5iTQEfx4xGrc0cA4TM5VwGdxTfyUU8wREsTuDi7GsWunFEKsPGZmHH+d/"
            "NNfDitK9esnG5QqdFgYEnKvWu9wHijoQHaEIKk+A6vCJrPRwfullOMPQV+R1ItRxLJY/"
            "BSO89tOBbD1+E+GMz9K0XRm1a3hegAmPq/nJSAjdyafKVk/8CXwFHCeMAlmFiI3iJ0Na/J4Qq6Xx5DW/"
            "bHcgum8LFDHrCT+GS1opoSLvoqC6C5k5vNkefBOYg3I3yd55XWYn5aaME0R63IyIyaf2WWYaljSlK73uI/"
            "GHBG9BLyr87X9p8ce1HlV0qWl";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA6AVb9xCR3AGrDVOJ2HSb\n"
            "XXwlXkh+PuYk0BH8eMRq3NHAOEzOVcBncU38lFPMERLE7g4uxrFrpxRCrDxmZhx/\n"
            "nfzTXw4rSvXrJxuUKnRYGBJyr1rvcB4o6EB2hCCpPgOrwiaz0cH7pZTjD0FfkdSL\n"
            "UcSyWPwUjvPbTgWw9fhPhjM/StF0ZtWt4XoAJj6v5yUgI3cmnylZP/Al8BRwnjAJ\n"
            "ZhYiN4idDWvyeEKul8eQ1v2x3ILpvCxQx6wk/hktaKaEi76KguguZObzZHnwTmIN\n"
            "yN8neeV1mJ+WmjBNEetyMiMmn9llmGpY0pSu97iPxhwRvQS8q/O1/afHHtR5VdKl\n"
            "pQIDAQAB\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIIEpAIBAAKCAQEA6AVb9xCR3AGrDVOJ2HSbXXwlXkh+PuYk0BH8eMRq3NHAOEzO\n"
            "VcBncU38lFPMERLE7g4uxrFrpxRCrDxmZhx/nfzTXw4rSvXrJxuUKnRYGBJyr1rv\n"
            "cB4o6EB2hCCpPgOrwiaz0cH7pZTjD0FfkdSLUcSyWPwUjvPbTgWw9fhPhjM/StF0\n"
            "ZtWt4XoAJj6v5yUgI3cmnylZP/Al8BRwnjAJZhYiN4idDWvyeEKul8eQ1v2x3ILp\n"
            "vCxQx6wk/hktaKaEi76KguguZObzZHnwTmINyN8neeV1mJ+WmjBNEetyMiMmn9ll\n"
            "mGpY0pSu97iPxhwRvQS8q/O1/afHHtR5VdKlpQIDAQABAoIBAEQ31ytURt596xIW\n"
            "/s+SsV+SMdZ/0AakWiyHLUlAzv6v+SlUg9I4qPFqcZoW7UgT0a0ApIYtAi70yQml\n"
            "FKJvPKJyBJ/NJbN7jLFJ7Y1x+bjSK8AdJ4s6guKOmYoNpFUkdSq3gGVq+JddqEkC\n"
            "+puA95mKELoTG+RQFKnjCH25jYAyH9gPF3JSZB4j0bVuPQMrmvD8DZqXQpY7vZ5o\n"
            "VvUr4pp4wi9aJGqOmjavTQ3+8WduIe+jv7u1UDMph/5Xki7iF5p8S2Xx7I9SLGjP\n"
            "pHhOt5iY9FRcS4PBBgh/1ONE50jI/yF+iQwyAkd39221RYEVc7NICpR3w5jX/hvl\n"
            "uozMw2ECgYEA9CVnC9gGaftV0Ee802vpK5iM0eVX0PTkr16B2xPzG87Ez8LAz8iT\n"
            "ldikBUqEam3iFKP2QgKjhKm09FkeWLG2fHPOb4p/51Br80CdVqRx78Wg5B+TfGUI\n"
            "bIKZ87vWpcVrIn2z/Ga1+UOUD66tbeCAjaW+SzpE6LWSdxuOgfk+4D0CgYEA80k/\n"
            "bD7picz6srcyJ7ZVZUoiiNLIroyQoGcqYWMj/njqWZ2aTUDEKFB4fswZj5MHsbXW\n"
            "MwE5CCleQe5Bb9TClqPjwFU3VxKVjHnYaVuric7IzmHfFNJVNE1DS61lm3981Hix\n"
            "YXPF52kinxO7fur5trMDI+OrKSgY1XfNqMiciYkCgYEAq6ispkAyakvkDziRRFnI\n"
            "LPXqdR36u5mkPMWHKgEB2phr+uQk6zNMyXvz/yBgr/AomHicCdePHxSvfWo3kwcI\n"
            "lsJZ8EUCHyKFdBgHSEcBMSPUNvEYosrqvFirZBq7Ff0TkcGuThXtdqAUN16K5AmI\n"
            "eY+Cl6QVlUUtAP/oF+ymT8UCgYEA6fMwKDm/fqI9GosMkh/Gsua9mvGHxkKp+XX7\n"
            "zTb3MAi6436pGU0E1Pe9R8Gheu2a4ovH9bhhTbXqTGB/ULk6fn4Pz66izvqyESpC\n"
            "r95VcOoHNF/tlCpHgUojQqz2HrhUJEYD3YDleQkjH/JrxTrU85nllSekOqmyZ+UT\n"
            "QhHYZnECgYB9rnXoxeKdwBfzeg3Adq8mr0mSdvi/OS4qpcDZF83C7LNzXMOtpBce\n"
            "xir5ly9huxbKjf0R1tZe/rcUUwi3r0P31nsShFC7BTIVE/Q1yyQgmVc1MgEiYV9o\n"
            "wKlDfTo3gtTZ5STXcZIBP7Yn0Lz3OZWnd7TtC+5eLRmu3qpWWPXD5Q==\n"
            "-----END RSA PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
    checkKey(publicKeyStrPkcs8, privateKeyStr);
}

TEST(DigitalSignatureKeyTest, RSA1024UnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ssh-rsa "
            "AAAAB3NzaC1yc2EAAAADAQABAAAAgQDGOeJLpy2mTG1pP3ata+"
            "rISathL6eNHpOSg96y9y3iMxDr0M6FsHNcYG4QWFZNr4UT2ulxJZe9c9RNrkKs4NzgL6e2t+"
            "YTk3eEnxlIOUAgTqwxH+OKwzSAA2pUlhYBPuv7u+LE01Avd+EQxxwa7yUPWVRCMMEM9kYJVYE0ryS0kQ==";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDGOeJLpy2mTG1pP3ata+rISath\n"
            "L6eNHpOSg96y9y3iMxDr0M6FsHNcYG4QWFZNr4UT2ulxJZe9c9RNrkKs4NzgL6e2\n"
            "t+YTk3eEnxlIOUAgTqwxH+OKwzSAA2pUlhYBPuv7u+LE01Avd+EQxxwa7yUPWVRC\n"
            "MMEM9kYJVYE0ryS0kQIDAQAB\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIICXgIBAAKBgQDGOeJLpy2mTG1pP3ata+rISathL6eNHpOSg96y9y3iMxDr0M6F\n"
            "sHNcYG4QWFZNr4UT2ulxJZe9c9RNrkKs4NzgL6e2t+YTk3eEnxlIOUAgTqwxH+OK\n"
            "wzSAA2pUlhYBPuv7u+LE01Avd+EQxxwa7yUPWVRCMMEM9kYJVYE0ryS0kQIDAQAB\n"
            "AoGBAIusz9Lk+tqaMeIecN5kE2yL1HHHpXVfnTZ5KxvJ5g/kUcEIQe2b1r25yPRF\n"
            "epKE4e2KrEEo7xH0ox0VdC/ixl8GL840vMObXy52SvtKx0UnWV/IOrAH1Fb6+bMx\n"
            "NdeA2GY2b+8UOlDWPtJDULp3AXKicmYY/vFBRHdZzZGNXX4BAkEA/UZtpXqaTAot\n"
            "Raau6aKpNhNlZnVJz7D42+DPgb+VMnhJdSrx5Lil5Ti2s3HtPjfHQa1zCd1IcL6z\n"
            "RFJmAdHWsQJBAMhb1uLwTC5CmFcpRROeX+/U3bLz/iLng6HqOOYvwbjMgsSit2gq\n"
            "3W2xyjUWYthWTtbvVWwfw+CbVCXET9r98+ECQQDoWiMe64H//loYCtluVx57EII2\n"
            "46TqV4WGWhCkDHk4SufBCLvPQ6JVzpi1uO+X453138SoqQva+ugE5r7ULoiRAkAk\n"
            "G8EPZyUjlSblVm+3iGxbya4mySg83CJx9MdKXML57gmHLJgHMupzEX6SsLUhrfRJ\n"
            "chqgAN2JRhUVNVr66ZehAkEAnXKD5k068dxPTLMUc1vQW9N5PZPX3yb/JuvUGVNO\n"
            "YCGhDief1DFrhx4mNINhxIuR2lbhhaPNHHg9Rfw4Z8cEMw==\n"
            "-----END RSA PRIVATE KEY-----";

    // 1024 RSA bits keys are not supported (min 2048)
    siodb::crypto::DigitalSignatureKey key;
    ASSERT_THROW(key.parseFromString(publicKeyStr), std::runtime_error);
    ASSERT_THROW(key.parseFromString(publicKeyStrPkcs8), std::runtime_error);
    ASSERT_THROW(key.parseFromString(privateKeyStr), std::runtime_error);
}

TEST(DigitalSignatureKeyTest, DSAUnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ssh-dss "
            "AAAAB3NzaC1kc3MAAACBAKvTA23FdMe2mKqDFUGLjKt5jem8XId/DYZSYZElr/"
            "48n1DMK2MQ1XUG9eVSBoUGgob8j4WFWU1iFZFpnTi8suUZnaxn/xLwgdymD6+i4sSTDul/"
            "67N7vv6Vd1pb17CP8yYkLqShppcOMo6xcPVBKiWi0SMjZxJ5mDOAW1XcE4+nAAAAFQDcGhBcZ9/"
            "J8oICYGrf9lQV0GqqOQAAAIAk2l0qSsxpVsIMmZ7kp88Q2IYeKyx+k5DDxBFGbAgzGF9B6cu4+"
            "5Q8VjsWwTahZE8hqjk3D26EGkRPtX4uSSxhlaC4WppraCn5h7sjfCRcVcoFVjLM8rjGDj2/"
            "cc7PFYrVpY8GyVUd7eHKE3JA3BqYMYLlmJ8kMP0xIk4z4UnMvwAAAIEAjc8BCIAquD3KDpz7pYKHs36tknbEEj"
            "iI4q29VVJW1BQ+bPBE6cynY+r9m+Qtu3g4qB7qWhVEv53NaRzQaNvctZuN+CLdEZ/TLLrDhKQxSfbq2/fR/"
            "MFXDWmp82u0+egDcNRxv0i9Myi1pMY7kge+LI38nlasIUHRQFKGFPcE7aE=";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIIBtzCCASsGByqGSM44BAEwggEeAoGBAKvTA23FdMe2mKqDFUGLjKt5jem8XId/\n"
            "DYZSYZElr/48n1DMK2MQ1XUG9eVSBoUGgob8j4WFWU1iFZFpnTi8suUZnaxn/xLw\n"
            "gdymD6+i4sSTDul/67N7vv6Vd1pb17CP8yYkLqShppcOMo6xcPVBKiWi0SMjZxJ5\n"
            "mDOAW1XcE4+nAhUA3BoQXGffyfKCAmBq3/ZUFdBqqjkCgYAk2l0qSsxpVsIMmZ7k\n"
            "p88Q2IYeKyx+k5DDxBFGbAgzGF9B6cu4+5Q8VjsWwTahZE8hqjk3D26EGkRPtX4u\n"
            "SSxhlaC4WppraCn5h7sjfCRcVcoFVjLM8rjGDj2/cc7PFYrVpY8GyVUd7eHKE3JA\n"
            "3BqYMYLlmJ8kMP0xIk4z4UnMvwOBhQACgYEAjc8BCIAquD3KDpz7pYKHs36tknbE\n"
            "EjiI4q29VVJW1BQ+bPBE6cynY+r9m+Qtu3g4qB7qWhVEv53NaRzQaNvctZuN+CLd\n"
            "EZ/TLLrDhKQxSfbq2/fR/MFXDWmp82u0+egDcNRxv0i9Myi1pMY7kge+LI38nlas\n"
            "IUHRQFKGFPcE7aE=\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN DSA PRIVATE KEY-----\n"
            "MIIBuwIBAAKBgQCr0wNtxXTHtpiqgxVBi4yreY3pvFyHfw2GUmGRJa/+PJ9QzCtj\n"
            "ENV1BvXlUgaFBoKG/I+FhVlNYhWRaZ04vLLlGZ2sZ/8S8IHcpg+vouLEkw7pf+uz\n"
            "e77+lXdaW9ewj/MmJC6koaaXDjKOsXD1QSolotEjI2cSeZgzgFtV3BOPpwIVANwa\n"
            "EFxn38nyggJgat/2VBXQaqo5AoGAJNpdKkrMaVbCDJme5KfPENiGHissfpOQw8QR\n"
            "RmwIMxhfQenLuPuUPFY7FsE2oWRPIao5Nw9uhBpET7V+LkksYZWguFqaa2gp+Ye7\n"
            "I3wkXFXKBVYyzPK4xg49v3HOzxWK1aWPBslVHe3hyhNyQNwamDGC5ZifJDD9MSJO\n"
            "M+FJzL8CgYEAjc8BCIAquD3KDpz7pYKHs36tknbEEjiI4q29VVJW1BQ+bPBE6cyn\n"
            "Y+r9m+Qtu3g4qB7qWhVEv53NaRzQaNvctZuN+CLdEZ/TLLrDhKQxSfbq2/fR/MFX\n"
            "DWmp82u0+egDcNRxv0i9Myi1pMY7kge+LI38nlasIUHRQFKGFPcE7aECFHBclhfL\n"
            "UvDpVHinBneUoLi2lsKx\n"
            "-----END DSA PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
    checkKey(publicKeyStrPkcs8, privateKeyStr);
}

TEST(DigitalSignatureKeyTest, ECDSA256UnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ecdsa-sha2-nistp256 "
            "AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBPtZ1FDqUSao4H3LXk+"
            "qHAHZOvitX8uToT6O+WOaK5GYNoMg8IYnZ5bu43DprBgvm4/cxmhCbRq1xA2ud+sHsKI=";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIIBSzCCAQMGByqGSM49AgEwgfcCAQEwLAYHKoZIzj0BAQIhAP////8AAAABAAAA\n"
            "AAAAAAAAAAAA////////////////MFsEIP////8AAAABAAAAAAAAAAAAAAAA////\n"
            "///////////8BCBaxjXYqjqT57PrvVV2mIa8ZR0GsMxTsPY7zjw+J9JgSwMVAMSd\n"
            "NgiG5wSTamZ44ROdJreBn36QBEEEaxfR8uEsQkf4vOblY6RA8ncDfYEt6zOg9KE5\n"
            "RdiYwpZP40Li/hp/m47n60p8D54WK84zV2sxXs7LtkBoN79R9QIhAP////8AAAAA\n"
            "//////////+85vqtpxeehPO5ysL8YyVRAgEBA0IABPtZ1FDqUSao4H3LXk+qHAHZ\n"
            "OvitX8uToT6O+WOaK5GYNoMg8IYnZ5bu43DprBgvm4/cxmhCbRq1xA2ud+sHsKI=\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN EC PRIVATE KEY-----\n"
            "MHcCAQEEIGeFYTZ0vZOsL21+ZhZnp3zFmn5cR2fS5Fjzb+8HdBusoAoGCCqGSM49\n"
            "AwEHoUQDQgAE+1nUUOpRJqjgfcteT6ocAdk6+K1fy5OhPo75Y5orkZg2gyDwhidn\n"
            "lu7jcOmsGC+bj9zGaEJtGrXEDa536wewog==\n"
            "-----END EC PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
    checkKey(publicKeyStrPkcs8, privateKeyStr);
}

TEST(DigitalSignatureKeyTest, ECDSA384UnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ecdsa-sha2-nistp384 "
            "AAAAE2VjZHNhLXNoYTItbmlzdHAzODQAAAAIbmlzdHAzODQAAABhBIyENwMb9DGMMgFJHtDBiFRh8swc1aNq59"
            "tubxXXGjZBkRv7GgKi1Yp8DewymiBMOh9y5Obb616B2OqOzWAgbo6Bt3UDg8KbTmWnZOiDGIEtGGcgmn+"
            "KheP5DdDmM9BaRw==";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIIBzDCCAWQGByqGSM49AgEwggFXAgEBMDwGByqGSM49AQECMQD/////////////\n"
            "/////////////////////////////v////8AAAAAAAAAAP////8wewQw////////\n"
            "//////////////////////////////////7/////AAAAAAAAAAD////8BDCzMS+n\n"
            "4j7n5JiOBWvj+C0ZGB2cbv6BQRIDFAiPUBOHWsZWOY2KLtGdKoXI7dPsKu8DFQCj\n"
            "NZJqoxmieh0AiWpnc6SCes2scwRhBKqHyiK+iwU3jrHHHvMgrXRuHTtii6ebmFn3\n"
            "QeCCVCo4VQLyXb9VKWw6VF44cnYKtzYX3kqWJixvXZ6Yv5KS3Cn49B29KJoUfOna\n"
            "MRO18LjACmCxzh1+gZ16Qx18kOoOXwIxAP//////////////////////////////\n"
            "/8djTYH0Ny3fWBoNskiwp3rs7BlqzMUpcwIBAQNiAASMhDcDG/QxjDIBSR7QwYhU\n"
            "YfLMHNWjaufbbm8V1xo2QZEb+xoCotWKfA3sMpogTDofcuTm2+tegdjqjs1gIG6O\n"
            "gbd1A4PCm05lp2TogxiBLRhnIJp/ioXj+Q3Q5jPQWkc=\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN EC PRIVATE KEY-----\n"
            "MIGkAgEBBDB26pz8dmI5xDsxeRWBcwPHcKqqSdnnJbyGCnQzz/MwASbAZsFxXYCQ\n"
            "P0dwFl/WR5+gBwYFK4EEACKhZANiAASMhDcDG/QxjDIBSR7QwYhUYfLMHNWjaufb\n"
            "bm8V1xo2QZEb+xoCotWKfA3sMpogTDofcuTm2+tegdjqjs1gIG6Ogbd1A4PCm05l\n"
            "p2TogxiBLRhnIJp/ioXj+Q3Q5jPQWkc=\n"
            "-----END EC PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
    checkKey(publicKeyStrPkcs8, privateKeyStr);
}

TEST(DigitalSignatureKeyTest, ECDSA521UnecryptedKeyTest)
{
    const std::string publicKeyStr =
            "ecdsa-sha2-nistp521 "
            "AAAAE2VjZHNhLXNoYTItbmlzdHA1MjEAAAAIbmlzdHA1MjEAAACFBAFnLr0iZW17XJOSHJGtBlM214vpm3gLh3"
            "OQX8KpV2N6BUhxCApUesC/ViQ/"
            "4eLC3nJZR0msb89RZjKt5ADmYXRKmAAJthvMGWxSAQz352RitZNXnrGefYDX+"
            "KElnyndYuSikwRZ6UvO57C9nnyUxhFOdLLSw8xgXdFXZJkVfk5KTO5Y9Q==";

    const std::string publicKeyStrPkcs8 =
            "-----BEGIN PUBLIC KEY-----\n"
            "MIICXDCCAc8GByqGSM49AgEwggHCAgEBME0GByqGSM49AQECQgH/////////////\n"
            "////////////////////////////////////////////////////////////////\n"
            "/////////zCBngRCAf//////////////////////////////////////////////\n"
            "///////////////////////////////////////8BEFRlT65YY4cmh+SmiGgtoVA\n"
            "7qLacluZsxXzuLSJkY7xCeFWGTlR7H6TexZSwL07sb8HNXPfiD0sNPHvRR/Ua1A/\n"
            "AAMVANCeiAApHLhTlsxnFzkyhKqg2mS6BIGFBADGhY4GtwQE6c2ePstmI5W0Qpxk\n"
            "gTkFP7Uh+CivYGtNPbqhS1537+dZKP4dwSei/6jeM0izwYVqQpv5fn4xwuW9ZgEY\n"
            "OSlqeJo7wARcil+0LH0b2Zj1RElXm0RoF6+9Fyc+ZiyX7nKZXvQmQMVQuQE/rQdh\n"
            "NTxwhqJywkCIvpR2n9FmUAJCAf//////////////////////////////////////\n"
            "////+lGGh4O/L5Zrf8wBSPcJpdA7tcm4iZxHrrtvtx6ROGQJAgEBA4GGAAQBZy69\n"
            "ImVte1yTkhyRrQZTNteL6Zt4C4dzkF/CqVdjegVIcQgKVHrAv1YkP+Hiwt5yWUdJ\n"
            "rG/PUWYyreQA5mF0SpgACbYbzBlsUgEM9+dkYrWTV56xnn2A1/ihJZ8p3WLkopME\n"
            "WelLzuewvZ58lMYRTnSy0sPMYF3RV2SZFX5OSkzuWPU=\n"
            "-----END PUBLIC KEY-----";

    const std::string privateKeyStr =
            "-----BEGIN EC PRIVATE KEY-----\n"
            "MIHcAgEBBEIBAOjSAJvSxZ1AdSmtmPwiu2NJGB7ozFGAJrS5en6rbxhmNwRYQ/04\n"
            "B31regj2SusWw4VhhFLDVVYso+swPzHd99ygBwYFK4EEACOhgYkDgYYABAFnLr0i\n"
            "ZW17XJOSHJGtBlM214vpm3gLh3OQX8KpV2N6BUhxCApUesC/ViQ/4eLC3nJZR0ms\n"
            "b89RZjKt5ADmYXRKmAAJthvMGWxSAQz352RitZNXnrGefYDX+KElnyndYuSikwRZ\n"
            "6UvO57C9nnyUxhFOdLLSw8xgXdFXZJkVfk5KTO5Y9Q==\n"
            "-----END EC PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
    checkKey(publicKeyStrPkcs8, privateKeyStr);
}

TEST(DigitalSignatureKeyTest, ED25519UnecryptedptedKeyTest)
{
    const std::string publicKeyStr =
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIDI9GcQvV3GiO2H7h/kUK7yuEs8ztpGscxvT90urbXnl";

    const std::string privateKeyStr =
            "-----BEGIN OPENSSH PRIVATE KEY-----\n"
            "b3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZW\n"
            "QyNTUxOQAAACAyPRnEL1dxojth+4f5FCu8rhLPM7aRrHMb0/dLq2155QAAAKAXZLo+F2S6\n"
            "PgAAAAtzc2gtZWQyNTUxOQAAACAyPRnEL1dxojth+4f5FCu8rhLPM7aRrHMb0/dLq2155Q\n"
            "AAAEAFVOuRDoAdpbg/Avz/ZQ3BcZUUKndBfHQsxvK/fFclLjI9GcQvV3GiO2H7h/kUK7yu\n"
            "Es8ztpGscxvT90urbXnlAAAAGGFsZXhleUBhbGV4ZXktVmlydHVhbEJveAECAwQF\n"
            "-----END OPENSSH PRIVATE KEY-----";

    checkKey(publicKeyStr, privateKeyStr);
}
