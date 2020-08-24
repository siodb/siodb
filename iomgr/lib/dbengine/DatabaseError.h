// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/MessageSeverity.h>
#include <siodb/common/utils/SiodbException.h>

namespace siodb::iomgr::dbengine {

/** Exception class for indicating general database errors */
class DatabaseError : public SiodbException {
protected:
    /**
     * Initializes object of class DatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    DatabaseError(MessageId errorCode, const char* errorMessage)
        : SiodbException(errorMessage, 4)
        , m_errorCode(static_cast<int>(errorCode))
    {
    }

    /**
     * Initializes object of class DatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    DatabaseError(MessageId errorCode, const std::string& errorMessage)
        : SiodbException(errorMessage, 4)
        , m_errorCode(static_cast<int>(errorCode))
    {
    }

public:
    /** Min and max I/O Error IDs */
    static constexpr std::pair<int, int> kIOErrorCodeRange = std::make_pair(0x80000000, 0x8fffffff);

    /** Min and max I/O Error IDs */
    static constexpr std::pair<int, int> kInternalErrorCodeRange =
            std::make_pair(0x90000000, 0x9fffffff);

    /**
     * Determines if message ID is in range.
     * @param messageId Message ID.
     * @param range A range.
     */
    template<typename MessageId, typename RangeBound>
    static bool isMessageIdInRange(
            MessageId messageId, const std::pair<RangeBound, RangeBound>& range) noexcept
    {
        return static_cast<RangeBound>(messageId) >= range.first
               && static_cast<RangeBound>(messageId) <= range.second;
    }

    /**
     * Returns error code.
     * @return error code.
     */
    int getErrorCode() const noexcept
    {
        return m_errorCode;
    }

private:
    /** Error code */
    const int m_errorCode;
};

/** Exception class for indicating user visible database errors */
class UserVisibleDatabaseError : public DatabaseError {
public:
    /**
     * Initializes object of class UserVisibleDatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    UserVisibleDatabaseError(MessageId errorCode, const char* errorMessage)
        : DatabaseError(errorMessage)
    {
    }

    /**
     * Initializes object of class UserVisibleDatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    UserVisibleDatabaseError(MessageId errorCode, const std::string& errorMessage)
        : DatabaseError(errorCode, errorMessage)
    {
    }
};

/** Exception class for indicating I/O errors */
class DatabaseIOError : public DatabaseError {
public:
    /**
     * Initializes object of class DatabaseIOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    DatabaseIOError(MessageId errorCode, const char* errorMessage)
        : DatabaseError(errorCode, errorMessage)
    {
    }

    /**
     * Initializes object of class DatabaseIOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    DatabaseIOError(MessageId errorCode, const std::string& errorMessage)
        : DatabaseError(errorCode, errorMessage)
    {
    }
};

/** Exception class for indicating internal logic errors */
class InternalDatabaseError : public DatabaseError {
public:
    /**
     * Initializes object of class InternalDatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    InternalDatabaseError(MessageId errorCode, const char* errorMessage)
        : DatabaseError(errorCode, errorMessage)
    {
    }

    /**
     * Initializes object of class InternalDatabaseError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    template<typename MessageId>
    InternalDatabaseError(MessageId errorCode, const std::string& errorMessage)
        : DatabaseError(errorCode, errorMessage)
    {
    }
};

/** Exception class for indicating multiple general database errors */
class CompoundDatabaseError : public SiodbException {
public:
    /** Single error record */
    struct ErrorRecord {
        ErrorRecord() noexcept
            : m_errorCode(0)
        {
        }

        /** Error code */
        int m_errorCode;

        /** Error message */
        std::string m_message;
    };

    /**
     * Initializes object of class CompoundDatabaseError.
     * @param errors Vector of errors.
     */
    explicit CompoundDatabaseError(const std::vector<ErrorRecord>& errors)
        : SiodbException("CompoundDatabaseError")
        , m_errors(errors)
    {
    }

    /**
     * Initializes object of class CompoundDatabaseError.
     * @param errors Vector of errors.
     */
    explicit CompoundDatabaseError(std::vector<ErrorRecord>&& errors)
        : SiodbException("CompoundDatabaseError")
        , m_errors(std::move(errors))
    {
    }

    /**
     * Returns list of errors.
     * @return List of errors.
     */
    const auto& getErrors() const
    {
        return m_errors;
    }

private:
    /** List of errors */
    const std::vector<ErrorRecord> m_errors;
};

}  // namespace siodb::iomgr::dbengine
