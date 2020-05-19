// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "StringScanner.h"

// STL headers
#include <stdexcept>

// Boost headers
#include <boost/algorithm/string/predicate.hpp>

namespace siodb::utils {

StringScanner::StringScanner(const char* text, std::size_t length)
    : m_begin(text)
    , m_end(text + length)
    , m_current(text)
{
    if (!text && length > 0) throw std::runtime_error("Scanner text is nullptr");
}

bool StringScanner::skipUntilWhitespace() noexcept
{
    if (!hasMoreData()) return false;
    while (!std::isspace(*m_current) && advance(1)) {
    }
    return hasMoreData();
}

bool StringScanner::skipWhitespaces() noexcept
{
    if (!hasMoreData()) return false;
    while (std::isspace(*m_current) && advance(1)) {
    }
    return hasMoreData();
}

std::size_t StringScanner::findInLine(const char* value, std::size_t size) const noexcept
{
    if (size == 0u) return 0u;

    const char* ptr = m_current;
    const int remainingSize = this->remainingSize() - size;
    for (int i = 0; i <= remainingSize && *ptr != '\n'; ++i, ++ptr)
        if (std::strncmp(ptr, value, size) == 0) return i;

    return std::string::npos;
}

bool StringScanner::advance(std::size_t size) noexcept
{
    if (remainingSize() < size) return false;
    m_current += size;
    return true;
}

bool StringScanner::read(void* dataPtr, std::size_t size) noexcept
{
    if (static_cast<std::size_t>(m_end - m_current) >= size) {
        memcpy(dataPtr, m_current, size);
        m_current += size;
        return true;
    }
    return false;
}

void StringScanner::setCurrent(const char* newCurrentData)
{
    if (!isDataInRange(newCurrentData))
        throw std::out_of_range("New current position is outside of scanner bounds");
    m_current = newCurrentData;
}

}  // namespace siodb::utils
