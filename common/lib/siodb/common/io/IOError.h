// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb {

/** I/O error */
class IOError : public std::runtime_error {
public:
    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    IOError(int errorCode, const char* errorMessage)
        : std::runtime_error(errorMessage)
        , m_errorCode(errorCode)
    {
    }

    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    IOError(int errorCode, const std::string& errorMessage)
        : std::runtime_error(errorMessage)
        , m_errorCode(errorCode)
    {
    }

    /**
     * Returns error code.
     */
    int getErrorCode() const noexcept
    {
        return m_errorCode;
    }

private:
    /** Error code */
    const int m_errorCode;
};

class FileReadError : public IOError {
public:
    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    FileReadError(int errorCode, const char* errorMessage)
        : IOError(errorCode, errorMessage)
    {
    }

    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    FileReadError(int errorCode, const std::string& errorMessage)
        : IOError(errorCode, errorMessage)
    {
    }
};

class FileWriteError : public IOError {
public:
    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    FileWriteError(int errorCode, const char* errorMessage)
        : IOError(errorCode, errorMessage)
    {
    }

    /**
     * Initializes object of class IOError.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    FileWriteError(int errorCode, const std::string& errorMessage)
        : IOError(errorCode, errorMessage)
    {
    }
};

}  // namespace siodb
