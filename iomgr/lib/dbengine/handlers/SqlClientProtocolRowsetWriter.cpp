// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SqlClientProtocolRowsetWriter.h"

// Project headers
#include "RequestHandlerSharedConstants.h"
#include "VariantOutput.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

SqlClientProtocolRowsetWriter::SqlClientProtocolRowsetWriter(siodb::io::OutputStream& connection)
    : m_rawOutput(connection, m_errorChecker)
    , m_codedOutput(&m_rawOutput)
{
}

void SqlClientProtocolRowsetWriter::beginRowset(
        iomgr_protocol::DatabaseEngineResponse& response, [[maybe_unused]] bool haveRows)
{
    protobuf::writeMessage(protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
            m_rawOutput, m_codedOutput);
}

void SqlClientProtocolRowsetWriter::endRowset()
{
    m_codedOutput.WriteVarint64(kNoMoreRows);
    m_rawOutput.CheckNoError();
}

void SqlClientProtocolRowsetWriter::writeRow(
        const std::vector<Variant>& values, const stdext::bitmask& nullMask)
{
    const auto rowLength =
            std::accumulate(values.cbegin(), values.cend(), static_cast<std::uint64_t>(0),
                    [](std::uint64_t left, const Variant& right) {
                        return left + getSerializedSize(right);
                    })
            + nullMask.size();

    m_codedOutput.WriteVarint64(rowLength);
    m_rawOutput.CheckNoError();

    if (!nullMask.empty()) {
        m_codedOutput.WriteRaw(nullMask.data(), nullMask.size());
        m_rawOutput.CheckNoError();
    }

    for (const auto& value : values) {
        writeVariant(value, m_codedOutput);
        m_rawOutput.CheckNoError();
    }
}

}  // namespace siodb::iomgr::dbengine
