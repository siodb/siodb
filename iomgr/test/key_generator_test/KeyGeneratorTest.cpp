// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/crypto/KeyGenerator.h"

// STL headers
#include <iomanip>
#include <iostream>
#include <sstream>

int main(int argc, char** argv)
{
    using namespace siodb::iomgr::dbengine::crypto;
    std::string seedModifier;
    if (argc > 1) seedModifier = argv[1];
    static const unsigned keyLengths[] = {64, 128, 192, 256, 384, 512};
    static const char* seeds[] = {"", "a", "ab", "abc", "jklmn##!??&^"};
    const char** seed = &seeds[0];
    for (auto keyLength : keyLengths) {
        std::string seedStr = *seed++;
        if (!seedModifier.empty()) seedStr += seedModifier;
        const auto key = generateCipherKey(keyLength, seedStr);
        std::ostringstream keyStr;
        for (auto byte : key) {
            keyStr << std::hex << std::setfill('0') << std::setw(2) << static_cast<short>(byte);
        }
        std::cout << keyLength << ": " << keyStr.str() << std::endl;
    }
    return 0;
}
