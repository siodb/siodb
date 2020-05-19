// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <type_traits>

namespace siodb::crypto {

/** Random bytes generator */
class RandomGenerator final {
public:
    /**
     * Initializes object of class RandomGenerator.
     * @throw std::runtime_error in case of error.
     */
    RandomGenerator();

    /**
     * Fills user provided data with random bytes.
     * @param data User data.
     * @param size User data size.
     * @throw OpenSslError in case of error.
     */
    void getRandomBytes(unsigned char* data, std::size_t size) const;

    /**
     * Return random trivial type filled with random bytes.
     * @return Generated random value.
     * @throw OpenSslError in case of error.
     */
    template<typename T>
    T getRandomValue() const
    {
        static_assert(std::is_trivial<T>::value, "T must be a trivial type");
        T value;
        getRandomBytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));
        return value;
    }
};

}  // namespace siodb::crypto
