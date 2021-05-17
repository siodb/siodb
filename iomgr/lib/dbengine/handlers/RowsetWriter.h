// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/stl_ext/bitmask.h>
#include <siodb/iomgr/shared/dbengine/Variant.h>

// STL headers
#include <string>
#include <vector>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::iomgr::dbengine {

/**
 * An object for outputting a rowset to a destination facility,
 * such as client connection stream or JSON.
 */
class RowsetWriter {
public:
    /** De-initializes object of class RowsetWriter. */
    virtual ~RowsetWriter() = default;

    /**
     * Begins a rowset.
     * @param response Database engine response.
     * @param haveRows Indication that there are rows in the respose.
     */
    virtual void beginRowset(
            iomgr_protocol::DatabaseEngineResponse& response, bool haveRows = true) = 0;

    /**
     * Ends a rowset.
     */
    virtual void endRowset() = 0;

    /**
     * Writes a row.
     * @param values The values to write.
     * @param nullMask Null values bitmask.
     */
    virtual void writeRow(const std::vector<Variant>& values, const stdext::bitmask& nullMask) = 0;
};

}  // namespace siodb::iomgr::dbengine
