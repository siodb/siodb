// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <sstream>

namespace stdext {

/**
 * String builder allows easier construction of strings via series of stream outputs.
 * @tparam CharT Character type.
 * @tparam Traints Charatcer traits.
 * @tparam Allocator Allocator type.
 */
template<class CharT, class Traits = std::char_traits<CharT>,
        class Allocator = std::allocator<CharT>>
class basic_string_builder {
public:
    /** Clears content */
    void clear()
    {
        const CharT ch = static_cast<CharT>(0);
        m_stream.str(&ch);
    }

    /**
     * Returns currently constructed string.
     * @return Constructed string.
     */
    std::basic_string<CharT, Traits, Allocator> str() const
    {
        return m_stream.str();
    }

    /**
     * Returns currently constructed string.
     * @return Constructed string.
     */
    operator std::basic_string<CharT, Traits, Allocator>() const
    {
        return m_stream.str();
    }

    /**
     * Sets new string.
     * @param s New string.
     */
    void str(const CharT* s)
    {
        m_stream.str(s);
    }

    /**
     * Sets new string.
     * @param s New string.
     */
    void str(const std::basic_string<CharT, Traits, Allocator>& s)
    {
        m_stream.str(s);
    }

    /**
     * Returns exception mask of the underlying stream.
     * @return Exception mask.
     */
    std::ios_base::iostate exceptions() const noexcept
    {
        return m_stream.exceptions();
    }

    /**
     * Set exception mask of the underlying stream.
     * @param except New exceptions mask.
     */
    void exceptions(std::ios_base::iostate except) noexcept
    {
        m_stream.exceptions(except);
    }

    /**
     * Appends string represenation of a value to the constructed string.
     * @param value A value to append.
     */
    template<typename V>
    basic_string_builder& operator<<(const V& value)
    {
        m_stream << value;
        return *this;
    }

    /**
     * Returns underlying stream state indicaion.
     * @return true is stream is in the "good" state, false otherwise.
     */
    operator bool() const noexcept
    {
        return static_cast<bool>(m_stream);
    }

protected:
    /** Underlying stream */
    std::basic_ostringstream<CharT, Traits, Allocator> m_stream;
};

/** String builder for the normal strings. */
using string_builder = basic_string_builder<char, std::char_traits<char>, std::allocator<char>>;

/** String builder for the wide-character strings. */
using wstring_builder =
        basic_string_builder<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;

#if __cplusplus > 201703L  // C++20 and above
/** String builder for the 8-bit character strings. */
using u8string_builder =
        basic_string_builder<char8_t, std::char_traits<char16_t>, std::allocator<char16_t>>;
#endif  // __cplusplus > 201703L

/** String builder for the 16-bit character strings. */
using u16string_builder =
        basic_string_builder<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>>;

/** String builder for the 32-bit character strings. */
using u32string_builder =
        basic_string_builder<char32_t, std::char_traits<char32_t>, std::allocator<char32_t>>;

}  // namespace stdext
