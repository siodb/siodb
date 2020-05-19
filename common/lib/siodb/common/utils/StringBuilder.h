// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <sstream>

namespace siodb::utils {

/**
 * String builder allows easier construction of string through series of stream outputs.
 * @tparam CharT Character type.
 * @tparam Traints Charatcer traits.
 * @tparam Allocator Allocator type.
 */
template<class CharT, class Traits = std::char_traits<CharT>,
        class Allocator = std::allocator<CharT>>
class BasicStringBuilder {
public:
    void clear()
    {
        const CharT ch = static_cast<CharT>(0);
        m_stream.str(&ch);
    }

    std::basic_string<CharT, Traits, Allocator> str() const
    {
        return m_stream.str();
    }

    operator std::basic_string<CharT, Traits, Allocator>() const
    {
        return m_stream.str();
    }

    void str(const CharT* s)
    {
        m_stream.str(s);
    }

    void str(const std::basic_string<CharT, Traits, Allocator>& s)
    {
        m_stream.str(s);
    }

    std::ios_base::iostate exceptions() const noexcept
    {
        return m_stream.exceptions();
    }

    void exceptions(std::ios_base::iostate except) noexcept
    {
        m_stream.exceptions(except);
    }

    template<typename V>
    BasicStringBuilder& operator<<(const V& value)
    {
        m_stream << value;
        return *this;
    }

    operator bool() const noexcept
    {
        return static_cast<bool>(m_stream);
    }

protected:
    std::basic_ostringstream<CharT, Traits, Allocator> m_stream;
};

using StringBuilder = BasicStringBuilder<char, std::char_traits<char>, std::allocator<char>>;
using WideStringBuilder =
        BasicStringBuilder<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;
using U16StringBuilder =
        BasicStringBuilder<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>>;
using U32StringBuilder =
        BasicStringBuilder<char32_t, std::char_traits<char32_t>, std::allocator<char32_t>>;

}  // namespace siodb::utils
