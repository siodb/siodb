// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"

// Common project headers
#include "../stl_ext/buffer.h"
#include "../utils/HelperMacros.h"

// STL headers
#include <streambuf>
#include <string>

namespace siodb::io {

/** 
 * BasicInputStreamStdStreamBuffer controls input from a Siodb input stream object.
 * @tparam CharT Character type.
 * @tparam Traits Character traits type.
 * @see https://habr.com/ru/post/326578/
 */
template<class CharT, class Traits = std::char_traits<CharT>>
class BasicInputStreamStdStreamBuffer : public std::basic_streambuf<CharT, Traits> {
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
     * Initializes object of class InputStreamStdStreamBuffer.
     * @param stream Underlying stream object.
     * @param bufferSize Buffer size.
     */
    BasicInputStreamStdStreamBuffer(InputStream& stream, std::size_t bufferSize)
        : m_stream(stream)
        , m_buffer(bufferSize)
    {
        auto dataEnd = m_buffer.data() + m_buffer.size();
        Base::setg(m_buffer.data(), dataEnd, dataEnd);
    }

    DECLARE_NONCOPYABLE(BasicInputStreamStdStreamBuffer);

protected:
    /**
     * Ensures that at least one character is available in the input area.
     * @return Returns the value of that character (converted to int_type with
     *         Traits::to_int_type(c)) on success or Traits::eof() on failure.
     */
    int_type underflow() override
    {
        if (Base::gptr() < Base::egptr()) return *Base::gptr();
        const auto start = Base::eback();
        const auto readCount = m_stream.read(start, sizeof(char_type) * m_buffer.size());
        Base::setg(start, start, start + readCount);
        return readCount > 0 ? *Base::gptr() : traits_type::eof();
    }

private:
    /** Underlying stream */
    InputStream& m_stream;
    /** Read buffer */
    stdext::buffer<char_type> m_buffer;
};

/** Normal character buffer */
using InputStreamStdStreamBuffer = BasicInputStreamStdStreamBuffer<char>;

/** Wide character buffer */
using InputStreamStdWStreamBuffer = BasicInputStreamStdStreamBuffer<wchar_t>;

}  // namespace siodb::io
