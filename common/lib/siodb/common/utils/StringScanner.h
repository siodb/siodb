// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstring>

// STL headers
#include <stdexcept>
#include <string>

namespace siodb::utils {

/**
 * Class for scanning and parsing a string.
 */
class StringScanner final {
public:
    /**
     * Initializes object of class StringScanner.
     * @param s A string.
     */
    explicit StringScanner(const std::string& s) noexcept
        : StringScanner(s.c_str(), s.length())
    {
    }

    /**
     * Initializes object of class StringScanner.
     * @param s A string.
     * @param length String length.
     * @throw runtime_error if @ref s is nullptr and @ref length is nonzero.
     */
    StringScanner(const char* text, std::size_t length);

    /**
     * Skips all data until whitespace from current position.
     * @return Indication whether data is available.
     */
    bool skipUntilWhitespace() noexcept;

    /**
     * Skips whitespace from current position.
     * @return Indication whether data is available.
     */
    bool skipWhitespaces() noexcept;

    /**
     * Return indication that data from current position is starts with specified string.
     * @param str String.
     * @param size String size.
     * @return true if data in current position starts with specified string, false otherwise.
     */
    bool startsWith(const char* value, std::size_t size) const noexcept
    {
        return (size <= remainingSize()) && std::strncmp(current(), value, size) == 0;
    }

    /**
     * Return indication that data from current position is starts with specified string.
     * @param value String.
     * @return true if data in current position starts with specified string, false otherwise.
     */
    bool startsWith(const char* str) const noexcept
    {
        return startsWith(str, std::strlen(str));
    }

    /**
     * Return indication that data from current position is starts with specified string.
     * @param str String.
     * @return true if data in current position starts with specified string, false otherwise.
     */
    bool startsWith(const std::string& str) const noexcept
    {
        return startsWith(str.c_str(), str.size());
    }

    /**
     * Searches specified string in scanner data from current position until end of data.
     * @param str Null-terminated string.
     * @return Next position or std::string::npos if string was not found.
     */
    std::size_t find(const char* str) const noexcept
    {
        std::string_view sv(m_current, remainingSize());
        return sv.find(str);
    }

    /**
     * Searches specified string in scanner data from current position until end of data.
     * @param str Null-terminated string.
     * @return Pointer where str is placed in data or nullptr if it not found. 
     */
    const char* findPtr(const char* str) const noexcept
    {
        const auto pos = find(str);
        return (pos != std::string::npos) ? m_current + pos : nullptr;
    }

    /**
     * Searches specified string in scanner data from current position until end of line.
     * @param str Null-terminated string.
     * @param size String size.
     * @return Value greater or equal to 0 means starting symbol from current position.
     * std::string::npos means specified value is not found.
     */
    std::size_t findInLine(const char* str, std::size_t size) const noexcept;

    /**
     * Searches specified string in scanner data from current position until end of line.
     * @param str String to search.
     * @return Value greater or equal to 0 means starting symbol from current position.
     * std::string::npos means specified value is not found.
     */
    std::size_t findInLine(const std::string& str) const noexcept
    {
        return findInLine(str.c_str(), str.size());
    }

    /**
     * Searches specified string in scanner data from current position until end of data.
     * @param str String to search.
     * @param size Length of string.
     * @return Pointer where str is placed in data or nullptr if it not found.
     */
    const char* findInLinePtr(const char* str, std::size_t size) const noexcept
    {
        const auto pos = findInLine(str, size);
        return (pos != std::string::npos) ? m_current + pos : nullptr;
    }

    /**
     * Searches specified string in scanner data from current position until end of data.
     * @param str String to search.
     * @return Pointer where str is placed in data or nullptr if it not found.
     */
    const char* findInLinePtr(const std::string& str) const noexcept
    {
        return findInLinePtr(str.c_str(), str.size());
    }

    /**
     * Moves current position forward by specified number of bytes.
     * @param size Number of bytes
     * @return true if move was successful, false otherwise.
     */
    bool advance(std::size_t size) noexcept;

    /**
     * Reads data from current position. Read data is copied into @ref dataPtr.
     * @param dataPtr Data pointer.
     * @param size Required data to be read.
     * @return true data was read successfully, false otherwise.
     */
    bool read(void* dataPtr, std::size_t size) noexcept;

    /**
     * Returns current scanner position.
     * @return Current scanner position.
     */
    std::size_t pos() const noexcept
    {
        return m_current - m_begin;
    }

    /**
     * Returns indication whether data is avaliable for read.
     * @return Indication whether data is available for read.
     */
    bool hasMoreData() const noexcept
    {
        return m_current < m_end;
    }

    /**
     * Returns data size.
     * @return Data size.
     */
    std::size_t size() const noexcept
    {
        return m_end - m_begin;
    }

    /**
     * Returns count of bytes until data end.
     * @return Count of bytes until data end.
     */
    std::size_t remainingSize() const noexcept
    {
        return m_end - m_current;
    }

    /**
     * Returns indication that specified data pointer is in [@m_begin @m_end range].
     * @param newCurrentData New current data.
     * @throw true if data pointer is in [@m_begin, @m_end range], false otherwise.
     */
    bool isDataInRange(const char* data) const noexcept
    {
        return data >= m_begin && data <= m_end;
    }

    /**
     * Returns data pointer without position offset.
     * @return Data pointer without position offset.
     */
    const char* data() noexcept
    {
        return m_begin;
    }

    /**
     * Returns data pointer with position offset.
     * @return Current data pointer with position offset.
     */
    const char* current() const noexcept
    {
        return m_current;
    }

    /**
     * Set current data by pointer.
     * @param newCurrentData New current data.
     * @throw std::out_of_range if newCurrentData is not in range of Scanner data.
     */
    void setCurrent(const char* newCurrentData);

private:
    /** Begin text pointer */
    const char* m_begin;

    /** End text pointer */
    const char* m_end;

    /** Current text pointer */
    const char* m_current;
};

}  // namespace siodb::utils
