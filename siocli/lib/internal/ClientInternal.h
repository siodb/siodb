// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Protobuf message headers
#include <siodb/common/proto/ColumnDataType.pb.h>

// STL headers
#include <ostream>

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>

namespace siodb::cli::detail {

// Default data widths
constexpr std::size_t kBoolDefaultDataWidth = 5;  // false/true
constexpr std::size_t kInt8DefaultDataWidth = 4;
constexpr std::size_t kUInt8DefaultDataWidth = 3;
constexpr std::size_t kInt16DefaultDataWidth = 6;
constexpr std::size_t kUInt16DefaultDataWidth = 5;
constexpr std::size_t kInt32DefaultDataWidth = 11;
constexpr std::size_t kUInt32DefaultDataWidth = 10;
constexpr std::size_t kInt64DefaultDataWidth = 20;
constexpr std::size_t kUInt64DefaultDataWidth = 19;
constexpr std::size_t kFloatDefaultDataWidth = 11;
constexpr std::size_t kDoubleDefaultDataWidth = 20;
constexpr std::size_t kTextDefaultDataWidth = 40;
constexpr std::size_t kNTextDefaultDataWidth = 40;
constexpr std::size_t kBinaryDefaultDataWidth = 40;

// 'Wed Jan 07 -524288'
constexpr std::size_t kDateDefaultDataWidth = 20;
// '11:30:35.123456789 AM'
constexpr std::size_t kTimeDefaultDataWidth = 20;
constexpr std::size_t kTimeWithTzDefaultDataWidth = 25;
// 'Wed Jan 07 -524288 11:30:35.123456789 AM'
constexpr std::size_t kTimestampDefaultDataWidth = 40;
constexpr std::size_t kTimestampWithTzDefaultDataWidth = 20;
constexpr std::size_t kDateIntervalWithTzDefaultDataWidth = 42;
constexpr std::size_t kTimeIntervalDefaultDataWidth = 42;
constexpr std::size_t kStructDefaultDataWidth = 32;
constexpr std::size_t kXmlDefaultDataWidth = 32;
constexpr std::size_t kJsonDefaultDataWidth = 32;
constexpr std::size_t kUuidDefaultDataWidth = 32;

// Read buffer size
constexpr std::size_t kLobReadBufferSize = 4096;

/**
 * Returns column data width in characters for give data type and column name length.
 * @param type Column type.
 * @param nameLength Column name length.
 */
std::size_t getColumnDataWidth(ColumnDataType type, std::size_t nameLength);

/**
 * Receives and prints column data.
 * @param is Input stream.
 * @param type Column type.
 * @param width Column width.
 * @param os Output stream.
 * @return true on success, false if error occurred.
 */
bool receiveAndPrintColumnData(google::protobuf::io::CodedInputStream& is, ColumnDataType type,
        std::size_t width, std::ostream& os);

/**
 * Prints NULL value.
 * @param width Column width.
 * @param os Output stream.
 */
void printNull(std::size_t width, std::ostream& os);

}  // namespace siodb::cli::detail
