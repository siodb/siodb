// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "RowsetWriter.h"

// Project common headers
#include <siodb/common/protobuf/ExtendedCodedOutputStream.h>
#include <siodb/common/protobuf/StreamOutputStream.h>

namespace siodb::iomgr::dbengine {

/**
 * An object for outputting a rowset to a client connection stream.
 */
class SqlClientProtocolRowsetWriter : public RowsetWriter {
public:
    /**
     * Initializes object of class SqlClientProtocolRowsetWriter.
     * @param connection Client connection.
     */
    explicit SqlClientProtocolRowsetWriter(siodb::io::OutputStream& connection);

    /**
     * Begins a rowset.
     * @param response Database engine response.
     * @param haveRows Indication that there are rows in the respose.
     */
    void beginRowset(iomgr_protocol::DatabaseEngineResponse& response, bool haveRows) override;

    /**
     * Ends a rowset.
     */
    void endRowset() override;

    /**
     * Writes a row.
     * @param values A values to write.
     * @param nullMask Null values bitmask.
     */
    void writeRow(const std::vector<Variant>& values, const stdext::bitmask& nullMask) override;

private:
    /** Error checker object */
    utils::DefaultErrorCodeChecker m_errorChecker;

    /** Raw data output stream */
    protobuf::StreamOutputStream m_rawOutput;

    /** Coded output stream */
    protobuf::ExtendedCodedOutputStream m_codedOutput;
};

}  // namespace siodb::iomgr::dbengine
