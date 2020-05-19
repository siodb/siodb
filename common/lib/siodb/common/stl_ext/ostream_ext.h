// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "cstdint_ext.h"

// STL headers
#include <ostream>

namespace stdext {

/**
 * Inserts string representation of value into stream os.
 * @param os Output stream.
 * @param value A value to be inserted.
 * @return Output stream.
 */
std::ostream& operator<<(std::ostream& os, const stdext::int128_t& value);

/**
 * Inserts string representation of value into stream os.
 * @param os Output stream.
 * @param value A value to be inserted.
 * @return Output stream.
 */
std::ostream& operator<<(std::ostream& os, const stdext::uint128_t& value);

}  // namespace stdext

namespace std {

// Help libdate to find them

inline std::ostream& operator<<(std::ostream& os, const stdext::int128_t& value)
{
    return stdext::operator<<(os, value);
}

inline std::ostream& operator<<(std::ostream& os, const stdext::uint128_t& value)
{
    return stdext::operator<<(os, value);
}

}  // namespace std
