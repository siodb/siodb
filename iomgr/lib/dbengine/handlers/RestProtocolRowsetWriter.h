// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "RowsetWriter.h"

// Common project headers
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>

namespace siodb::iomgr::dbengine {

/**
 * An object for outputting a rowset to a REST protocol stream.
 */
class RestProtocolRowsetWriter : public RowsetWriter {
public:
    /**
     * Initializes object of class RestProtocolRowsetWriter.
     * @param connection Client connection stream.
     */
    explicit RestProtocolRowsetWriter(siodb::io::OutputStream& connection);

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
    /** Client connection */
    siodb::io::OutputStream& m_connection;

    /** Chunked output stream */
    siodb::io::BufferedChunkedOutputStream m_chunkedOutput;

    /** JSON writer */
    siodb::io::JsonWriter m_jsonWriter;

    /** Field names */
    std::vector<std::string> m_fieldNames;

    /** Indication that comma before next row is required */
    bool m_needCommaBeforeRow;
};

}  // namespace siodb::iomgr::dbengine
