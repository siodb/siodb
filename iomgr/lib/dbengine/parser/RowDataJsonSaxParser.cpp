// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RowDataJsonSaxParser.h"

// Project headers
#include "JsonParserError.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// STL headers
#include <sstream>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

namespace siodb::iomgr::dbengine::parser {

RowDataJsonSaxParser::RowDataJsonSaxParser(std::size_t rowCountLimit,
        std::unordered_map<unsigned, std::string>& columnNames,
        std::vector<std::vector<std::pair<unsigned, Variant>>>& values)
    : m_rowCountLimit(validateRowCountLimit(rowCountLimit))
    , m_columnNames(columnNames)
    , m_values(values)
    , m_columnIdCounter(0)
    , m_state(ParserState::kRowArray)
{
}

// --- internals ---

bool RowDataJsonSaxParser::null()
{
    checkParserState(ParserState::kColumnValue, "null value");
    addValue(Variant());
    return true;
}

bool RowDataJsonSaxParser::boolean(bool val)
{
    checkParserState(ParserState::kColumnValue, "boolean value");
    addValue(Variant(val));
    return true;
}

bool RowDataJsonSaxParser::number_integer(number_integer_t val)
{
    checkParserState(ParserState::kColumnValue, "integer number value");
    addValue(Variant(val));
    return true;
}

bool RowDataJsonSaxParser::number_unsigned(number_unsigned_t val)
{
    checkParserState(ParserState::kColumnValue, "unsigned integer number value");
    addValue(Variant(val));
    return true;
}

bool RowDataJsonSaxParser::number_float(number_float_t val, [[maybe_unused]] const string_t& s)
{
    checkParserState(ParserState::kColumnValue, "floating point number value");
    addValue(Variant(val));
    return true;
}

bool RowDataJsonSaxParser::string(string_t& val)
{
    checkParserState(ParserState::kColumnValue, "string value");
    addValue(Variant(std::move(val)));
    return true;
}

bool RowDataJsonSaxParser::binary(binary_t& val)
{
    checkParserState(ParserState::kColumnValue, "binary value");
    BinaryValue bv(val.data(), val.data() + val.size());
    addValue(Variant(std::move(bv)));
    return true;
}

bool RowDataJsonSaxParser::start_object([[maybe_unused]] std::size_t elements)
{
    checkParserState(ParserState::kRow, "begin of object");
    m_row.clear();
    m_state = ParserState::kColumnName;
    return true;
}

bool RowDataJsonSaxParser::key(string_t& val)
{
    checkParserState(ParserState::kColumnName, "key");
    if (!isValidDatabaseObjectName(val)) {
        std::ostringstream err;
        err << "Invalid column name: " << val;
        throw JsonParserError(err.str());
    }
    m_columnName = std::move(val);
    m_state = ParserState::kColumnValue;
    return true;
}

bool RowDataJsonSaxParser::end_object()
{
    checkParserState(ParserState::kColumnName, "end of object");
    if (m_values.size() < m_rowCountLimit) {
        m_values.push_back(std::move(m_row));
        m_state = ParserState::kRow;
        return true;
    }
    throw JsonParserError("Too many rows");
}

bool RowDataJsonSaxParser::start_array([[maybe_unused]] std::size_t elements)
{
    checkParserState(ParserState::kRowArray, "begin of array");
    m_state = ParserState::kRow;
    return true;
}

bool RowDataJsonSaxParser::end_array()
{
    checkParserState(ParserState::kRow, "end of array");
    m_state = ParserState::kFinished;
    return true;
}

bool RowDataJsonSaxParser::parse_error(
        std::size_t position, const std::string& last_token, const nlohmann::detail::exception& ex)
{
    std::ostringstream err;
    err << "at position " << position << ", near '" << last_token << "': " << ex.what();
    throw JsonParserError(ex.what());
}

void RowDataJsonSaxParser::addValue(Variant&& value)
{
    boost::to_upper(m_columnName);
    auto it = m_columnNameToIdMapping.find(m_columnName);
    if (it == m_columnNameToIdMapping.end()) {
        it = m_columnNameToIdMapping.emplace(m_columnName, ++m_columnIdCounter).first;
        m_columnNames.emplace(it->second, it->first);
    }
    const auto columnId = it->second;
    auto valueIt = std::find_if(m_row.begin(), m_row.end(),
            [columnId](const auto& e) noexcept { return e.first == columnId; });
    if (valueIt == m_row.end())
        m_row.emplace_back(columnId, std::move(value));
    else {
        std::ostringstream err;
        err << "Duplicate column '" << m_columnName << "' in the row #" << (m_values.size() + 1);
        throw JsonParserError(err.str());
    }
    m_state = ParserState::kColumnName;
}

std::size_t RowDataJsonSaxParser::validateRowCountLimit(std::size_t rowCountLimit)
{
    if (rowCountLimit > 0) return rowCountLimit;
    throw std::invalid_argument("RowDataJsonSaxParser: Invalid row count limit");
}

void RowDataJsonSaxParser::checkParserState(ParserState requiredState, const char* event) const
{
    if (m_state == requiredState) return;
    std::ostringstream err;
    err << "Unexpected " << event;
    throw JsonParserError(err.str());
}

}  // namespace siodb::iomgr::dbengine::parser
