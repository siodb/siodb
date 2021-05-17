// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include <siodb/iomgr/shared/dbengine/Variant.h>

// JSON library issue workaround
// https://github.com/nlohmann/json/issues/2755
#include <cstdio>
#include <string>
#undef EOF
static constexpr auto EOF = std::char_traits<char>::eof();

// JSON library
#include <nlohmann/json.hpp>

namespace siodb::iomgr::dbengine::parser {

/** SAX style parser for the row data in the JSON format. */
class RowDataJsonSaxParser : public nlohmann::json_sax<nlohmann::json> {
public:
    /**
     * Initalizes object of class RowDataJsonSaxParser.
     * @param rowCountLimit Maximum number of rows accepped.
     * @param columnNames Column names container.
     * @param values Column values container.
     */
    explicit RowDataJsonSaxParser(std::size_t rowCountLimit,
            std::unordered_map<unsigned, std::string>& columnNames,
            std::vector<std::vector<std::pair<unsigned, Variant>>>& values);

protected:
    /**
     * @brief a null value was read
     * @return whether parsing should proceed
     */
    bool null() override;

    /**
     * @brief a boolean value was read
     * @param[in] val  boolean value
     * @return whether parsing should proceed
     */
    bool boolean(bool val) override;

    /**
     * @brief an integer number was read
     * @param[in] val  integer value
     * @return whether parsing should proceed
     */
    bool number_integer(number_integer_t val) override;

    /**
     * @brief an unsigned integer number was read
     * @param[in] val  unsigned integer value
     * @return whether parsing should proceed
     */
    bool number_unsigned(number_unsigned_t val) override;

    /**
     * @brief an floating-point number was read
     * @param[in] val  floating-point value
     * @param[in] s    raw token value
     * @return whether parsing should proceed
    */
    bool number_float(number_float_t val, const string_t& s) override;

    /**
     * @brief a string was read
     * @param[in] val  string value
     * @return whether parsing should proceed
     * @note It is safe to move the passed string.
     */
    bool string(string_t& val) override;

    /**
     * @brief a binary string was read
     * @param[in] val  binary value
     * @return whether parsing should proceed
     * @note It is safe to move the passed binary.
     */
    bool binary(binary_t& val) override;

    /**
     * @brief the beginning of an object was read
     * @param[in] elements  number of object elements or -1 if unknown
     * @return whether parsing should proceed
     * @note binary formats may report the number of elements
     */
    bool start_object(std::size_t elements) override;

    /**
     * @brief an object key was read
     * @param[in] val  object key
     * @return whether parsing should proceed
     * @note It is safe to move the passed string.
     */
    bool key(string_t& val) override;

    /**
     * @brief the end of an object was read
     * @return whether parsing should proceed
     */
    bool end_object() override;

    /**
     * @brief the beginning of an array was read
     * @param[in] elements  number of array elements or -1 if unknown
     * @return whether parsing should proceed
     * @note binary formats may report the number of elements
     */
    bool start_array(std::size_t elements) override;

    /**
     * @brief the end of an array was read
     * @return whether parsing should proceed
     */
    bool end_array() override;

    /**
     * @brief a parse error occurred
     * @param[in] position    the position in the input where the error occurs
     * @param[in] last_token  the last read token
     * @param[in] ex          an exception object describing the error
     * @return whether parsing should proceed (must return false)
     */
    bool parse_error(std::size_t position, const std::string& last_token,
            const nlohmann::detail::exception& ex) override;

private:
    /** Parser states */
    enum class ParserState {
        kRowArray,
        kRow,
        kColumnName,
        kColumnValue,
        kFinished,
    };

private:
    /**
     * Adds value to the current row.
     * @param value A value.
     */
    void addValue(Variant&& value);

    /**
     * Validates row count limit.
     * @param rowCountLimit A row count limit to validate.
     * @return The same rowCountLimit if it is valid.
     * @throw std::invalid_argument if rowCountLimit is invalid.
     */
    static std::size_t validateRowCountLimit(std::size_t rowCountLimit);

    /**
     * Ensures parser is in the expected state.
     * @param requiredState Required state.
     * @param event Event description.
     * @throw JsonParserError if parser is in the invalid state.
     */
    void checkParserState(ParserState requiredState, const char* event) const;

private:
    /** Row count limit */
    const std::size_t m_rowCountLimit;

    /** Column names container */
    std::unordered_map<unsigned, std::string>& m_columnNames;

    /** Column values container */
    std::vector<std::vector<std::pair<unsigned, Variant>>>& m_values;

    /** Column name to numeric column identifier mapping */
    std::unordered_map<std::string, unsigned> m_columnNameToIdMapping;

    /** Numeric column identifier counter */
    unsigned m_columnIdCounter;

    /** Parser state */
    ParserState m_state;

    /** Saved column name */
    std::string m_columnName;

    /** Saved row */
    std::vector<std::pair<unsigned, Variant>> m_row;
};

}  // namespace siodb::iomgr::dbengine::parser
