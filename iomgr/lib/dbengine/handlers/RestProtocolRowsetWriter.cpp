// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RestProtocolRowsetWriter.h"

// Project headers
#include "JsonOutput.h"
#include "RequestHandlerSharedConstants.h"
#include "VariantOutput.h"

// Common project headers
#include <siodb/common/net/HttpStatus.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/StreamOutputStream.h>
#include <siodb/common/stl_ext/system_error_ext.h>

namespace siodb::iomgr::dbengine {

RestProtocolRowsetWriter::RestProtocolRowsetWriter(siodb::io::OutputStream& connection)
    : m_connection(connection)
    , m_chunkedOutput(kJsonChunkSize, m_connection)
    , m_jsonWriter(m_chunkedOutput)
    , m_needCommaBeforeRow(false)
{
}

void RestProtocolRowsetWriter::beginRowset(
        iomgr_protocol::DatabaseEngineResponse& response, bool haveRows)
{
    // Save field names
    m_fieldNames.clear();
    for (int i = 0, n = response.column_description_size(); i < n; ++i) {
        const auto& column = response.column_description(i);
        m_fieldNames.push_back(column.name());
    }

    // Send response message
    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
    response.set_rest_status_code(haveRows ? net::HttpStatus::kOk : net::HttpStatus::kNotFound);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    // Write JSON prolog
    writeGetJsonProlog(response.rest_status_code(), m_jsonWriter);
}

void RestProtocolRowsetWriter::endRowset()
{
    writeJsonEpilog(m_jsonWriter);
    if (m_chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RestProtocolRowsetWriter::writeRow(
        const std::vector<Variant>& values, [[maybe_unused]] const stdext::bitmask& nullMask)
{
    if (SIODB_LIKELY(m_needCommaBeforeRow))
        m_jsonWriter.writeComma();
    else
        m_needCommaBeforeRow = true;

    m_jsonWriter.writeObjectBegin();
    bool needCommaBeforeColumn = false;
    for (std::size_t i = 0, n = m_fieldNames.size(); i != n; ++i) {
        if (SIODB_LIKELY(needCommaBeforeColumn))
            m_jsonWriter.writeComma();
        else
            needCommaBeforeColumn = true;
        m_jsonWriter.writeFieldName(m_fieldNames[i]);
        writeVariant(values.at(i), m_jsonWriter);
    }
    m_jsonWriter.writeObjectEnd();
}

}  // namespace siodb::iomgr::dbengine
