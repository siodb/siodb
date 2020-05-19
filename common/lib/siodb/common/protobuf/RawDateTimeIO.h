// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../data/RawDateTime.h"

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>

namespace siodb::protobuf {

/**
 * Reads RawDateTime from a coded stream.
 * @param is Input stream.
 * @param rawDateTime Destination raw datetime object.
 * @return true on success, false on error.
 */
bool readRawDateTime(google::protobuf::io::CodedInputStream& is, RawDateTime& rawDateTime);

/**
 * Writes RawDateTime to the coded stream.
 * NOTE: error must be checked externally on the underlying stream (e.g. file stream).
 * @param os Output stream.
 * @param rawDateTime Source raw datetime object.
 */
void writeRawDateTime(google::protobuf::io::CodedOutputStream& os, const RawDateTime& rawDateTime);

}  // namespace siodb::protobuf
