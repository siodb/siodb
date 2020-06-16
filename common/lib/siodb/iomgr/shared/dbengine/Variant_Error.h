// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Variant_Type.h"

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine {

/** A base class for any variant value logic error */
class VariantLogicError : public std::logic_error {
protected:
    /**
     * Initializes object of class VariantLogicError.
     * @param what Explanatory message.
     */
    explicit VariantLogicError(const char* what)
        : std::logic_error(what)
    {
    }

    /**
     * Initializes object of class VariantLogicError.
     * @param what Explanatory message.
     */
    explicit VariantLogicError(const std::string& what)
        : std::logic_error(what)
    {
    }
};

/** Variant value type cast error exception class */
class VariantTypeCastError : public VariantLogicError {
public:
    /**
     * Initializes object of class VariantTypeCastError.
     * @param sourceValueType Source value type.
     * @param destValueType Destination value type.
     */
    VariantTypeCastError(VariantType sourceValueType, VariantType destValueType)
        : VariantLogicError(makeErrorMessage(sourceValueType, destValueType))
        , m_sourceValueType(sourceValueType)
        , m_destValueType(destValueType)
    {
    }

    /**
     * Initializes object of class VariantTypeCastError.
     * @param sourceValueType Source value type.
     * @param destValueType Destination value type.
     */
    VariantTypeCastError(VariantType sourceValueType, VariantType destValueType, const char* reason)
        : VariantLogicError(makeErrorMessage(sourceValueType, destValueType, reason))
        , m_sourceValueType(sourceValueType)
        , m_destValueType(destValueType)
    {
    }

    /**
     * Returns source value type.
     * @return Source value type.
     */
    VariantType getSourceValueType() const noexcept
    {
        return m_sourceValueType;
    }

    /**
     * Returns destination value type.
     * @return Destination value type.
     */
    VariantType getDestValueType() const noexcept
    {
        return m_destValueType;
    }

private:
    /**
     * Creates error message.
     * @param sourceValueType Source value type.
     * @param destValueType Destination value type.
     * @param reason Optional reason description.
     * @return Error message
     */
    static std::string makeErrorMessage(
            VariantType sourceValueType, VariantType destValueType, const char* reason = nullptr);

private:
    /** Source value type */
    const VariantType m_sourceValueType;

    /** Destination value type */
    const VariantType m_destValueType;
};

/** Thrown when Variant type doesn't fit operation requirements */
class WrongVariantTypeError : public VariantLogicError {
public:
    /**
     * Initializes object of class WrongVariantTypeError.
     * @param sourceValueType Source value type.
     */
    WrongVariantTypeError(VariantType sourceValueType, const char* reason = nullptr)
        : VariantLogicError(makeErrorMessage(sourceValueType, reason))
        , m_sourceValueType(sourceValueType)
    {
    }

private:
    /**
     * Creates error message.
     * @param sourceValueType Source value type.
     * @param reason Optional reason description.
     * @return Error message.
     */
    static std::string makeErrorMessage(VariantType sourceValueType, const char* reason);

private:
    /** Source value type */
    const VariantType m_sourceValueType;
};

/** A base class for any variant value runtime error */
class VariantRuntimeError : public std::runtime_error {
protected:
    /**
     * Initializes object of class VariantLogicError.
     * @param what Explanatory message.
     */
    explicit VariantRuntimeError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class VariantLogicError.
     * @param what Explanatory message.
     */
    explicit VariantRuntimeError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

/** Thrown when variant value serialization fails. */
class VariantSerializationError : public VariantRuntimeError {
public:
    /**
     * Initializes object of class VariantDeserializartionError.
     * @param what Explanatory string.
     */
    explicit VariantSerializationError(const char* what)
        : VariantRuntimeError(what)
    {
    }

    /**
     * Initializes object of class VariantDeserializartionError.
     * @param what Explanatory string.
     */
    explicit VariantSerializationError(const std::string& what)
        : VariantRuntimeError(what)
    {
    }
};

/** Thrown when variant value deserialization fails. */
class VariantDeserializationError : public VariantRuntimeError {
public:
    /**
     * Initializes object of class VariantDeserializartionError.
     * @param what Explanatory string.
     */
    explicit VariantDeserializationError(const char* what)
        : VariantRuntimeError(what)
    {
    }

    /**
     * Initializes object of class VariantDeserializartionError.
     * @param what Explanatory string.
     */
    explicit VariantDeserializationError(const std::string& what)
        : VariantRuntimeError(what)
    {
    }
};

}  // namespace siodb::iomgr::dbengine
