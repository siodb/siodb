// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>
#include <vector>

// Protobuf message headers
#include <siodb/common/proto/CommonTypes.pb.h>

namespace siodb {

/**
 * Exception thrown when siodb server responsed with error 
 */
class SqlQueryException : public std::runtime_error {
public:
    /**
     * Initializes object of class SqlQueryException
     * @param errors Errors.
     */
    explicit SqlQueryException(std::vector<siodb::StatusMessage>&& errors) noexcept
        : std::runtime_error("SQL error")
        , m_errors(std::move(errors))
    {
    }

    /**
     * Returns collected errors
     * @return eErrors
     */
    const std::vector<siodb::StatusMessage>& getErrors() const
    {
        return m_errors;
    }

private:
    /** Errors */
    std::vector<siodb::StatusMessage> m_errors;
};

}  // namespace siodb
