// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdlib>

namespace stdext {

/**
 * Deletes object using CRT free() function.
 * @tparam T Object type.
 */
template<class T>
struct free_deleter {
    void operator()(T* object) const noexcept
    {
        std::free(object);
    }
};

}  // namespace stdext
