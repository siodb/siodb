// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <ostream>
#include <string>
#include <variant>

namespace siodb {

/** Effictiently holds either string constant or mutable string value. */
class MutableOrConstantString {
public:
    /**
      * Initializes object of class MutableOrConstantString.
      * @param s Source string.
      */
    MutableOrConstantString(const char* s) noexcept
        : m_value(s)
    {
    }

    /**
      * Initializes object of class MutableOrConstantString.
      * @param s Source string.
      */
    MutableOrConstantString(const std::string& s)
        : m_value(s)
    {
    }

    /**
      * Initializes object of class MutableOrConstantString.
      * @param s Source string.
      */
    MutableOrConstantString(std::string&& s) noexcept
        : m_value(std::move(s))
    {
    }

    /**
     * Return indication that underlying value is constant.
     * @return true if underlying value is constant, false otherwise.
     */
    bool isConstantValue() const noexcept
    {
        return std::holds_alternative<const char*>(m_value);
    }

    /**
     * Returns underlying value as C string.
     * @return C string.
     */
    const char* c_str() const noexcept
    {
        return isConstantValue() ? std::get<const char*>(m_value)
                                 : std::get<std::string>(m_value).c_str();
    }

    /**
     * Returns underlying value as constant string.
     * @return Constant string.
     */
    const std::string& asConstantString()
    {
        return asMutableString();
    }

    /**
     * Returns underlying value as constant string.
     * @return Constant string.
     */
    std::string& asMutableString()
    {
        if (isConstantValue()) m_value = std::string(std::get<const char*>(m_value));
        return std::get<std::string>(m_value);
    }

private:
    /** Underlying value */
    std::variant<std::string, const char*> m_value;
};

/**
 * Stream output operator for MutableOrConstantString.
 * @param os Output stream.
 * @param v A value.
 */
inline std::ostream& operator<<(std::ostream& os, const MutableOrConstantString& v)
{
    os << v.c_str();
    return os;
}

}  // namespace siodb