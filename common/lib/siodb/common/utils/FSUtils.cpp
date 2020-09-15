// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FSUtils.h"

// Project headers
#include "../stl_wrap/filesystem_wrapper.h"

namespace siodb::utils {

void clearDir(const std::string& path)
{
    for (fs::directory_iterator endIt, it(path); it != endIt; ++it)
        fs::remove_all(it->path());
}

bool clearDir(const std::string& path, boost::system::error_code& errorCode)
{
    boost::system::error_code ec;
    for (fs::directory_iterator endIt, it(path); it != endIt; ++it) {
        if (!fs::remove_all(it->path(), ec)) {
            errorCode = ec;
            return false;
        }
    }
    return true;
}

}  // namespace siodb::utils