// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <streambuf>

namespace siodb::io {

/** 
 * MemoryStdStreamBuffer controls input from a memory buffer.
 * @tparam CharT Character type.
 * @tparam Traits Character traits type.
 * @see https://stackoverflow.com/a/10615477/1540501
 */
template<class CharT, class Traits = std::char_traits<CharT>>
class BasicMemoryStdStreamBuffer : public std::basic_streambuf<CharT, Traits> {
private:
    /** Base class type */
    using Base = std::basic_streambuf<CharT, Traits>;

public:
    // Standard types
    using char_type = typename Base::char_type;
    using traits_type = typename Base::traits_type;
    using int_type = typename Base::int_type;
    using pos_type = typename Base::pos_type;
    using off_type = typename Base::off_type;

public:
    /**
     * Initializs object of class MemoryStdStreamBuffer.
     * @param buffer Memory buffer address.
     * @param size Memory buffer size.
     * @throw std::invalid_argument if buffer is nullptr, or zero size, or buffer size
     *                              is not proportional to charater type size.
     */
    BasicMemoryStdStreamBuffer(void* buffer, std::size_t size)
    {
        if (buffer == nullptr || size == 0 || size % sizeof(char_type) != 0)
            throw std::invalid_argument("Invalid buffer");
        Base::setg(reinterpret_cast<char_type*>(buffer), reinterpret_cast<char_type*>(buffer),
                reinterpret_cast<char_type*>(reinterpret_cast<std::uint8_t*>(buffer) + size));
    }
};

/** Normal character buffer */
using MemoryStdStreamBuffer = BasicMemoryStdStreamBuffer<char>;

/** Wide character buffer */
using MemoryStdWStreamBuffer = BasicMemoryStdStreamBuffer<wchar_t>;

}  // namespace siodb::io
