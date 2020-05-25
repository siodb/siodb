// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Variant_Error.h"
#include "lob/BlobStream.h"
#include "lob/ClobStream.h"

// Common project headers
#include <siodb/common/data/RawDateTime.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/OptionalDeleter.h>

// CRT headers
#include <cstring>
#include <ctime>

// STL headers
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace siodb::iomgr::dbengine {

/** Variant value representation */
class Variant {
public:
    /** Maximum binary length that can be converted to string */
    static constexpr std::size_t kMaxBinarySizeConvertibleToString = 0x8000;

    /** Maximum string value length in bytes */
    static constexpr std::size_t kMaxStringValueLength = 0xffff;

    /** Maximum binary value length in bytes */
    static constexpr std::size_t kMaxBinaryValueLength = 0xffff;

    /** True boolean value string */
    static constexpr const char* kTrueString = "true";

    /** False boolean value string */
    static constexpr const char* kFalseString = "false";

    /** Default date/time format */
    static constexpr const char* kDefaultDateTimeFormat = "%Y-%m-%d %H:%M:%S";

    /** Default date format */
    static constexpr const char* kDefaultDateFormat = "%Y-%m-%d";

    /** Default time format */
    static constexpr const char* kDefaultTimeFormat = "%H:%M:%S";

public:
    /** Date/time value creation tag */
    struct AsDateTime {
    };

    /**
     * Special smart pointer with controllable resource ownership.
     * @tparam T Underlying data type.
     */
    template<class T>
    using OptionalOwnershipUniquePtr = std::unique_ptr<T, utils::OptionalDeleter<T>>;

public:
    /** Initializes object of class Variant */
    Variant() noexcept
        : m_valueType(VariantType::kNull)
    {
        m_value.m_ptr = nullptr;
    }

    /**
     * Initializes object of class Variant from other object.
     * @param src Source object.
     */
    Variant(const Variant& src);

    /**
     * Initializes object of class Variant from other object.
     * @param src Source object.
     */
    Variant(Variant&& src) noexcept
        : m_valueType(src.m_valueType)
    {
        m_value.m_ptr = src.m_value.m_ptr;
        src.m_valueType = VariantType::kNull;
        src.m_value.m_ptr = nullptr;
    }

    /** Initializes object of class Variant. */
    Variant(std::nullptr_t) noexcept
        : m_valueType(VariantType::kNull)
    {
        m_value.m_ptr = nullptr;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(bool value) noexcept
        : m_valueType(VariantType::kBool)
    {
        m_value.m_bool = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::int8_t value) noexcept
        : m_valueType(VariantType::kInt8)
    {
        m_value.m_i8 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::uint8_t value) noexcept
        : m_valueType(VariantType::kUInt8)
    {
        m_value.m_ui8 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::int16_t value) noexcept
        : m_valueType(VariantType::kInt16)
    {
        m_value.m_i16 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::uint16_t value) noexcept
        : m_valueType(VariantType::kUInt16)
    {
        m_value.m_ui16 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::int32_t value) noexcept
        : m_valueType(VariantType::kInt32)
    {
        m_value.m_i32 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::uint32_t value) noexcept
        : m_valueType(VariantType::kUInt32)
    {
        m_value.m_ui32 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::int64_t value) noexcept
        : m_valueType(VariantType::kInt64)
    {
        m_value.m_i64 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::uint64_t value) noexcept
        : m_valueType(VariantType::kUInt64)
    {
        m_value.m_ui64 = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(float value) noexcept
        : m_valueType(VariantType::kFloat)
    {
        m_value.m_float = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(double value) noexcept
        : m_valueType(VariantType::kDouble)
    {
        m_value.m_double = value;
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(const RawDateTime& value)
        : m_valueType(VariantType::kDateTime)
    {
        m_value.m_dt = new RawDateTime(value);
    }

    /**
     * Initializes object of class Variant.
     * @param s A datetime or date only string.
     * @param tag A date/time creation tag.
     * @param format Date/time string format.
     * @throw std::invalid_argument if @ref s cannot be parsed as date/time.
     */
    Variant(const char* s, [[maybe_unused]] AsDateTime tag, const char* format = nullptr)
        : m_valueType(VariantType::kDateTime)
    {
        const auto len = std::strlen(s);
        if (!format) format = getDateTimeFormat(len);
        m_value.m_dt = new RawDateTime(stringToDateTime(s, len, format));
    }

    /**
     * Initializes object of class Variant.
     * @param s A datetime or date only string.
     * @param tag A date/time creation tag.
     * @param format Date/time string format.
     * @throw std::invalid_argument if @ref s cannot be parsed as date/time.
     */
    Variant(const std::string& s, [[maybe_unused]] AsDateTime tag, const char* format = nullptr)
        : m_valueType(VariantType::kDateTime)
    {
        m_value.m_dt = new RawDateTime(stringToDateTime(s, format ? format : getDateTimeFormat(s)));
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false.
     */
    Variant(const char* value, bool allowNull = false);

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(const std::string& value)
        : m_valueType(VariantType::kString)
    {
        m_value.m_string = new std::string(value);
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::string&& value)
        : m_valueType(VariantType::kString)
    {
        m_value.m_string = new std::string(std::move(value));
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false.
     */
    Variant(std::string* value, bool allowNull = false);

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(const std::optional<std::string>& value)
        : m_valueType(value ? VariantType::kString : VariantType::kNull)
    {
        if (value) m_value.m_string = new std::string(*value);
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::optional<std::string>&& value)
        : m_valueType(value ? VariantType::kString : VariantType::kNull)
    {
        if (value) m_value.m_string = new std::string(std::move(*value));
    }

    /**
     * Initializes object of class Variant.
     * @param buffer Data buffer.
     * @param size Buffer size.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false
     *        or size is nonzero and value is nullptr.
     */
    Variant(const void* value, std::size_t size, bool allowNull = false);

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(const BinaryValue& value)
        : m_valueType(VariantType::kBinary)
    {
        m_value.m_binary = new BinaryValue(value);
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(BinaryValue&& value)
        : m_valueType(VariantType::kBinary)
    {
        m_value.m_binary = new BinaryValue(std::move(value));
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false.
     */
    Variant(BinaryValue* value, bool allowNull = false);

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(const std::optional<BinaryValue>& value)
        : m_valueType(value ? VariantType::kBinary : VariantType::kNull)
    {
        if (value) m_value.m_binary = new BinaryValue(*value);
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     */
    Variant(std::optional<BinaryValue>&& value)
        : m_valueType(value ? VariantType::kBinary : VariantType::kNull)
    {
        if (value) m_value.m_binary = new BinaryValue(std::move(*value));
    }

    /**
     * Initializes object of class Variant.
     * @param value A value.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false.
     */
    Variant(ClobStream* value, bool allowNull = false);

    /**
     * Initializes object of class Variant.
     * @param value A value.
     * @param allowNull Indication that null value is allowed.
     * @throw std::invalid_argument if @ref value is nullptr and allowNull is false.
     */
    Variant(BlobStream* value, bool allowNull = false);

    /** De-initializes object of class Variant */
    ~Variant()
    {
        clear();
    }

    /**
     * Returns value type.
     * @return Value type.
     */
    VariantType getValueType() const noexcept
    {
        return m_valueType;
    }

    /**
     * Returns value type name.
     * @return Value type name.
     */
    const char* getValueTypeName() const noexcept
    {
        return getVariantTypeName(m_valueType);
    }

    /**
     * Checks if this value type is null type
     * @return true if value type is null, false otherwise.
     */
    bool isNull() const noexcept
    {
        return m_valueType == VariantType::kNull;
    }

    /**
     * * Checks if this value type is string
     * @return true if value type is string, false otherwise.
     */
    bool isString() const noexcept
    {
        return m_valueType == VariantType::kString;
    }

    /**
     * Checks if this value type is BLOB.
     * @return true if value type is BLOB, false otherwise.
     */
    bool isBlob() const noexcept
    {
        return m_valueType == VariantType::kBlob;
    }

    /**
     * Checks if this value type is CLOB.
     * @return true if value type is CLOB, false otherwise.
     */
    bool isClob() const noexcept
    {
        return m_valueType == VariantType::kClob;
    }

    /**
     * Checks if this value type is binary.
     * @return true if value type is string, false otherwise.
     */
    bool isBinary() const noexcept
    {
        return m_valueType == VariantType::kBinary;
    }

    /**
     * Checks if this value type is datetime
     * @return true if value type is DateTime, false otherwise.
     */
    bool isDateTime() const noexcept
    {
        return m_valueType == VariantType::kDateTime;
    }

    /**
     * Checks if this value type is boolean.
     * @return true if value is boolean, false otherwise
     */
    bool isBool() const noexcept
    {
        return m_valueType == VariantType::kBool;
    }

    /**
     * Checks if this value type is numeric type
     * @return true if value is numeric, false otherwise
     */
    bool isNumeric() const noexcept
    {
        return isNumericType(m_valueType);
    }

    /**
     * Checks if this value type is integer
     * @return true if value is integer, false otherwise
     */
    bool isInteger() const noexcept
    {
        return isIntegerType(m_valueType);
    }

    /**
     * Checks if this value type is floating point type.
     * @return true if value type is floating point, false otherwise
     */
    bool isFloatingPoint() const noexcept
    {
        return isFloatingPointType(m_valueType);
    }

    /**
     * Returns boolean value. Does not check actual value type.
     * @return Boolean value.
     */
    bool getBool() const noexcept
    {
        return m_value.m_bool;
    }

    /**
     * Returns 8-bit signed integer value. Does not check actual value type.
     * @return 8-bit signed integer value.
     */
    std::int8_t getInt8() const noexcept
    {
        return m_value.m_i8;
    }

    /**
     * Returns 8-bit unsigned integer value. Does not check actual value type.
     * @return 8-bit unsigned integer value.
     */
    std::uint8_t getUInt8() const noexcept
    {
        return m_value.m_ui8;
    }

    /**
     * Returns 16-bit signed integer value. Does not check actual value type.
     * @return 16-bit signed integer value.
     */
    std::int16_t getInt16() const noexcept
    {
        return m_value.m_i16;
    }

    /**
     * Returns 16-bit unsigned integer value. Does not check actual value type.
     * @return 16-bit unsigned integer value.
     */
    std::uint16_t getUInt16() const noexcept
    {
        return m_value.m_ui16;
    }

    /**
     * Returns 32-bit signed integer value. Does not check actual value type.
     * @return 32-bit signed integer value.
     */
    std::int32_t getInt32() const noexcept
    {
        return m_value.m_i32;
    }

    /**
     * Returns 32-bit unsigned integer value. Does not check actual value type.
     * @return 32-bit unsigned integer value.
     */
    std::uint32_t getUInt32() const noexcept
    {
        return m_value.m_ui32;
    }

    /**
     * Returns 64-bit signed integer value. Does not check actual value type.
     * @return 64-bit signed integer value.
     */
    std::int64_t getInt64() const noexcept
    {
        return m_value.m_i64;
    }

    /**
     * Returns 64-bit unsigned integer value. Does not check actual value type.
     * @return 64-bit unsigned integer value.
     */
    std::uint64_t getUInt64() const noexcept
    {
        return m_value.m_ui64;
    }

    /**
     * Returns 32-bit IEEE-754 floating point value. Does not check actual value type.
     * @return 32-bit IEEE-754 floating point value.
     */
    float getFloat() const noexcept
    {
        return m_value.m_float;
    }

    /**
     * Returns 64-bit IEEE-754 floating point value. Does not check actual value type.
     * @return 64-bit IEEE-754 floating point value.
     */
    double getDouble() const noexcept
    {
        return m_value.m_double;
    }

    /**
     * Returns date/time. Does not check actual value type.
     * @return date/time.
     */
    RawDateTime& getDateTime() noexcept
    {
        return *m_value.m_dt;
    }

    /**
     * Returns date/time. Does not check actual value type.
     * @return date/time.
     */
    const RawDateTime& getDateTime() const noexcept
    {
        return *m_value.m_dt;
    }

    /**
     * Returns string value. Does not check actual value type.
     * @return String value.
     */
    std::string& getString() noexcept
    {
        return *m_value.m_string;
    }

    /**
     * Returns string value. Does not check actual value type.
     * @return String value.
     */
    const std::string& getString() const noexcept
    {
        return *m_value.m_string;
    }

    /**
     * Returns binary value. Does not check actual value type.
     * @return Binary value.
     */
    BinaryValue& getBinary() noexcept
    {
        return *m_value.m_binary;
    }

    /**
     * Returns binary value. Does not check actual value type.
     * @return Binary value.
     */
    const BinaryValue& getBinary() const noexcept
    {
        return *m_value.m_binary;
    }

    /**
     * Returns CLOB stream. Does not check actual value type.
     * @return CLOB stream.
     */
    ClobStream& getClob() noexcept
    {
        return *m_value.m_clob;
    }

    /**
     * Returns CLOB stream. Does not check actual value type.
     * @return CLOB stream.
     */
    const ClobStream& getClob() const noexcept
    {
        return *m_value.m_clob;
    }

    /**
     * Returns BLOB stream. Does not check actual value type.
     * @return BLOB stream.
     */
    BlobStream& getBlob() noexcept
    {
        return *m_value.m_blob;
    }

    /**
     * Returns BLOB stream. Does not check actual value type.
     * @return BLOB stream.
     */
    const BlobStream& getBlob() const noexcept
    {
        return *m_value.m_blob;
    }

    /**
     * Returns boolean value. Attempts to cast current value type into required value type.
     * @return Boolean value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    bool asBool() const;

    /**
     * Returns 8-bit signed integer value. Attempts to cast current value type into required
     * value type.
     * @return 8-bit signed integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int8_t asInt8() const;

    /**
     * Returns 16-bit unsigned integer value. Attempts to cast current value type into required
     * value type.
     * @return 16-bit unsigned integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int8_t asUInt8() const;

    /**
     * Returns 16-bit signed integer value. Attempts to cast current value type into required
     * value type.
     * @return 16-bit signed integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int16_t asInt16() const;

    /**
     * Returns 16-bit unsigned integer value. Attempts to cast current value type into required
     * value type.
     * @return 16-bit unsigned integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint16_t asUInt16() const;

    /**
     * Returns 32-bit signed integer value. Attempts to cast current value type into required
     * value type.
     * @return 32-bit signed integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int32_t asInt32() const;

    /**
     * Returns 32-bit unsigned integer value. Attempts to cast current value type into required
     * value type.
     * @return 32-bit unsigned integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint32_t asUInt32() const;

    /**
     * Returns 64-bit signed integer value. Attempts to cast current value type into required
     * value type.
     * @return 64-bit signed integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int64_t asInt64() const;

    /**
     * Returns 64-bit unsigned integer value. Attempts to cast current value type into required
     * value type.
     * @return 64-bit unsigned integer value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint64_t asUInt64() const;

    /**
     * Returns 32-bit IEEE-754 floating point value. Attempts to cast current value type into
     * required value type.
     * @return 32-bit IEEE-754 floating point value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    float asFloat() const;

    /**
     * Returns 64-bit IEEE-754 floating point value. Attempts to cast current value type into
     * required value type.
     * @return 64-bit IEEE-754 floating point value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    double asDouble() const;

    /**
     * Returns date/time value. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion from string to date time. If not specified or nullptr,
     *               default format will be used.
     * @return date/time value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime asDateTime(const char* format = nullptr) const;

    /**
     * Returns string value. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If not specified
     *               or nullptr, default format will be used.
     * @return String object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<std::string> asString(const char* format = nullptr)
    {
        const auto res = asStringInternal(format);
        return OptionalOwnershipUniquePtr<std::string>(
                stdext::as_mutable_ptr(res.first), !res.second);
    }

    /**
     * Returns string value. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If not specified
     *               or nullptr, default format will be used.
     * @return Optional string object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::optional<std::string> asOptionalString(const char* format = nullptr)
    {
        if (isNull()) return std::nullopt;
        const auto p = asString(format);
        if (p.get_deleter().isOwner())
            return std::move(*p);  // This is copy of string
        else
            return *p;  // This is string itself.
    }

    /**
     * Returns string value. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If not specified
     *               or nullptr, default format will be used.
     * @return String object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<const std::string> asString(const char* format = nullptr) const
    {
        const auto res = asStringInternal(format);
        return OptionalOwnershipUniquePtr<const std::string>(res.first, !res.second);
    }

    /**
     * Returns binary value. Attempts to cast current value type into required value type.
     * @return Binary value object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<BinaryValue> asBinary()
    {
        const auto res = asBinaryInternal();
        return OptionalOwnershipUniquePtr<BinaryValue>(
                stdext::as_mutable_ptr(res.first), !res.second);
    }

    /**
     * Returns binary value. Attempts to cast current value type into required value type.
     * @return Binary value object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<const BinaryValue> asBinary() const
    {
        const auto res = asBinaryInternal();
        return OptionalOwnershipUniquePtr<const BinaryValue>(res.first, !res.second);
    }

    /**
     * Returns CLOB stream. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If not specified
     *               or nullptr, default format will be used.
     * @return CLOB stream object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<ClobStream> asClob(const char* format = nullptr)
    {
        const auto res = asClobInternal(format);
        return OptionalOwnershipUniquePtr<ClobStream>(
                stdext::as_mutable_ptr(res.first), !res.second);
    }

    /**
     * Returns CLOB stream. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If not specified
     *               or nullptr, default format will be used.
     * @return CLOB stream object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<const ClobStream> asClob(const char* format = nullptr) const
    {
        const auto res = asClobInternal(format);
        return OptionalOwnershipUniquePtr<const ClobStream>(res.first, !res.second);
    }

    /**
     * Returns BLOB stream. Attempts to cast current value type into required value type.
     * @return BLOB stream object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<BlobStream> asBlob()
    {
        const auto res = asBlobInternal();
        return OptionalOwnershipUniquePtr<BlobStream>(
                stdext::as_mutable_ptr(res.first), !res.second);
    }

    /**
     * Returns BLOB stream. Attempts to cast current value type into required value type.
     * @return BLOB stream object.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    OptionalOwnershipUniquePtr<const BlobStream> asBlob() const
    {
        const auto res = asBlobInternal();
        return OptionalOwnershipUniquePtr<const BlobStream>(res.first, !res.second);
    }

    /**
     * Returns memory size in bytes required to serialize this value.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Serializes this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address of the next byte after last written byte.
     * @throw VariantSerialiationError if serialization failed.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const;

    /**
     * Deserializes value.
     * @param buffer Memory buffer address.
     * @param length Data length.
     * @return Number of consumed bytes.
     * @throw VariantDeserialiationError if deserializalization failed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Clears current value, assigns null */
    void clear() noexcept;

    /** Releases current value, assigns null */
    void release() noexcept
    {
        m_valueType = VariantType::kNull;
    }

    /**
     * Dumps value to a stream.
     * @param os Output stream.
     */
    void dump(std::ostream& os) const;

    /**
     * Swap content of this object with other object.
     * @param other Other object.
     */
    void swap(Variant& other) noexcept
    {
        if (SIODB_UNLIKELY(&other == this)) return;
        std::swap(m_valueType, other.m_valueType);
        if constexpr (sizeof(m_value.m_ptr) > sizeof(m_value.m_ui64))
            std::swap(m_value.m_ptr, other.m_value.m_ptr);
        else
            std::swap(m_value.m_ui64, other.m_value.m_ui64);
    }

    /**
     * Copy assignment operator.
     * @param value A source value.
     * @return Reference to this object.
     */
    Variant& operator=(const Variant& src)
    {
        if (SIODB_LIKELY(&src != this)) {
            Variant copy(src);
            clear();
            swap(copy);
        }
        return *this;
    }

    /**
     * Move assignment operator.
     * @param src A source value.
     * @return Reference to this object.
     */
    Variant& operator=(Variant&& src) noexcept
    {
        if (SIODB_LIKELY(&src != this)) {
            clear();
            swap(src);
        }
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(bool value) noexcept
    {
        clear();
        m_valueType = VariantType::kBool;
        m_value.m_bool = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::int8_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kInt8;
        m_value.m_i8 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::uint8_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kUInt8;
        m_value.m_ui8 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::int16_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kInt16;
        m_value.m_i16 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::uint16_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kUInt16;
        m_value.m_ui16 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::int32_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kInt32;
        m_value.m_i32 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::uint32_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kUInt32;
        m_value.m_ui32 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::int64_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kInt64;
        m_value.m_i64 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::uint64_t value) noexcept
    {
        clear();
        m_valueType = VariantType::kUInt64;
        m_value.m_ui64 = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(float value) noexcept
    {
        clear();
        m_valueType = VariantType::kFloat;
        m_value.m_float = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(double value) noexcept
    {
        clear();
        m_valueType = VariantType::kDouble;
        m_value.m_double = value;
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const RawDateTime& value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const char* value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const std::string& value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::string&& value) noexcept
    {
        Variant tmp(std::move(value));
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::string* value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const std::optional<std::string>& value)
    {
        if (value)
            *this = *value;
        else
            clear();
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::optional<std::string>&& value) noexcept
    {
        if (value)
            *this = std::move(*value);
        else
            clear();
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const BinaryValue& value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(BinaryValue&& value) noexcept
    {
        Variant tmp(std::move(value));
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(BinaryValue* value)
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(const std::optional<BinaryValue>& value)
    {
        if (value)
            *this = *value;
        else
            clear();
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(std::optional<BinaryValue>&& value) noexcept
    {
        if (value)
            *this = std::move(*value);
        else
            clear();
        return *this;
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(ClobStream* value) noexcept
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Assignment operator.
     * @param value A value.
     * @return Reference to this object.
     */
    Variant& operator=(BlobStream* value) noexcept
    {
        Variant tmp(value);
        return *this = std::move(tmp);
    }

    /**
     * Returns null value.
     * @return Null variant value.
     */
    static const Variant& null() noexcept
    {
        return m_nullValue;
    }

    /**
     * Returns empty string value.
     * @return Empty string value.
     */
    static const Variant& emptyString() noexcept
    {
        return m_emptyStringValue;
    }

    /**
     * Equality operator.
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false otherwise.
     */
    bool operator==(const Variant& other) const noexcept;

    /**
     * Non-equality operator.
     * NOTE: BLOBs and CLOBs are always non-equal.
     * @param other Other object.
     * @return true if objects are not equal, false otherwise.
     */
    bool operator!=(const Variant& other) const noexcept;

    /**
     * Less operator.
     * NOTE: BLOBs and CLOBs are compared by length then by stream address.
     * @param other Other object.
     * @return true if this object is less than other object, false otherwise.
     */
    bool operator<(const Variant& other) const noexcept;

    /**
     * Less or equal operator.
     * NOTE: BLOBs and CLOBs are compared by length then by stream address.
     * @param other Other object.
     * @return true if this object is less than or equal to other object, false otherwise.
     */
    bool operator<=(const Variant& other) const noexcept;

    /**
     * Greater operator.
     * NOTE: BLOBs and CLOBs are compared by length then by stream address.
     * @param other Other object.
     * @return true if this object is greater than other object, false otherwise.
     */
    bool operator>(const Variant& other) const noexcept;

    /**
     * Less or equal operator.
     * NOTE: BLOBs and CLOBs are compared by length then by stream address.
     * @param other Other object.
     * @return true if this object is greater than or equal to other object, false otherwise.
     */
    bool operator>=(const Variant& other) const noexcept;

    /**
     * Unary minus operator.
     * @return Value after operation.
     * @throw WrongVariantTypeError in case of non numeric value
     */
    Variant operator-() const;

    /**
     * Unary plus operator.
     * @return Value after operation.
     * @throw WrongVariantTypeError in case of non numeric value
     */
    Variant operator+() const;

    /**
     * Unary complement operator.
     * @return Value after operation.
     * @throw WrongVariantTypeError in case of non integer value
     */
    Variant operator~() const;

    /**
     * Sum operator.
     * @param other Other Value
     * @return Sum of values, or concatenation of strings.
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator+(const Variant& other) const;

    /**
     * Subtraction operator.
     * @param other Other Value
     * @return Result of substration of values.
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator-(const Variant& other) const;

    /**
     * Multiplication operator.
     * @param other Other Value
     * @return Result of multiplication of values.
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator*(const Variant& other) const;

    /**
     * Division operator.
     * @param other Other Value
     * @return Result of division of values.
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator/(const Variant& other) const;

    /**
     * Modulo operator.
     * @param other Other value
     * @return Remainder.
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator%(const Variant& other) const;

    /**
     * Bitwise or operator.
     * @param other Other value
     * @return Bitwise or operation result
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator|(const Variant& other) const;

    /**
     * Bitwise and operator.
     * @param other Other value
     * @return Bitwise and operation result
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator&(const Variant& other) const;

    /**
     * Bitwise XOR operator.
     * @param other Other value
     * @return Bitwise xor operation result
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator^(const Variant& other) const;

    /**
     * Bitwise left shift operator.
     * @param other Other value
     * @return Left shifted value
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator<<(const Variant& count) const;

    /**
     * Bitwise right shift operator.
     * @param other Other value
     * @return Right shifted value
     * @throw VariantTypeCastError if it was not possible to cast one of values to the resulting value type
     */
    Variant operator>>(const Variant& count) const;

    /**
     * Concatenates values. Non string values type casted to string.
     * @param other Other value
     * @return Concatenated values
     * @throw VariantTypeCastError if it was not possible to cast one of values to the string
     */
    Variant concatenate(const Variant& other) const;

    /**
     * Checks equality of compatible values bool, numeric, strings, datetime, binary types are compared.
     * null, BLOB, CLOB never equal).
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false or uncompatible otherwise.
     * @throw VariantTypeCastError if comparison is impossible
     */
    bool compatibleEqual(const Variant& other) const;

    /**
     * Checks if this value is less than other.
     * Compatible values bool, numeric, strings, datetime, binary types are compared.
     * null, BLOB, CLOB never equal).
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false or uncompatible otherwise.
     * @throw VariantTypeCastError if comparison is impossible
     */
    bool compatibleLess(const Variant& other) const;

    /**
     * Checks if this value is less than or equal to other.
     * Compatible values bool, numeric, strings, datetime, binary types are compared.
     * null, BLOB, CLOB never equal).
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false or uncompatible otherwise.
     * @throw VariantTypeCastError if comparison is impossible
     */
    bool compatibleLessOrEqual(const Variant& other) const;

    /**
     * Checks if this value is greater than to other.
     * Compatible values bool, numeric, strings, datetime, binary types are compared.
     * null, BLOB, CLOB never equal).
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false or uncompatible otherwise.
     * @throw VariantTypeCastError if comparison is impossible
     */
    bool compatibleGreater(const Variant& other) const;

    /**
     * Checks if this value is greater than or equal to other.
     * Compatible values bool, numeric, strings, datetime, binary types are compared.
     * null, BLOB, CLOB never equal).
     * NOTE: BLOBs and CLOBs are never equal.
     * @param other Other object.
     * @return true if objects are equal, false or uncompatible otherwise.
     * @throw VariantTypeCastError if comparison is impossible
     */
    bool compatibleGreaterOrEqual(const Variant& other) const;

    /**
     * Checks if this number positive(>0)
     * @return true if number is negative, false otherwise
     * @throw WrongVariantTypeError in case of non numeric value
     */
    bool isPositive() const;

    /**
     * Checks if this number negative(<0)
     * @return true if number is negative, false otherwise
     * @throw WrongVariantTypeError in case of non numeric value
     */
    bool isNegative() const;

    /**
     * Checks if this number is zero(==0)
     * @return true if number is negative, false otherwise
     * @throw WrongVariantTypeError in case of non numeric value
     */
    bool isZero() const;

private:
    /** Value representation */
    union Value {
        /** Boolean value */
        bool m_bool;

        /** 8-bit signed integer value */
        std::int8_t m_i8;

        /** 8-bit unsigned integer value */
        std::uint8_t m_ui8;

        /** 16-bit signed integer value */
        std::int16_t m_i16;

        /** 16-bit unsigned integer value */
        std::uint16_t m_ui16;

        /** 32-bit signed integer value */
        std::int32_t m_i32;

        /** 32-bit unsigned integer value */
        std::uint32_t m_ui32;

        /** 64-bit signed integer value */
        std::int64_t m_i64;

        /** 64-bit unsigned integer value */
        std::uint64_t m_ui64;

        /** 32-bit IEEE-754 floating point value */
        float m_float;

        /** 64-bit IEEE-754 floating point value */
        double m_double;

        /** String value */
        std::string* m_string;

        /** Binary value */
        BinaryValue* m_binary;

        /** CLOB value */
        ClobStream* m_clob;

        /** BLOB value */
        BlobStream* m_blob;

        /** Date/time value */
        RawDateTime* m_dt;

        /** Supplementary pointer */
        void* m_ptr;
    };

private:
    /**
     * Returns string value. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to string is some cases. If nullptr,
     *               default format will be used.
     * @return Pair (String value, ownership flag).
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::pair<const std::string*, bool> asStringInternal(const char* format) const;

    /**
     * Returns binary value. Attempts to cast current value type into required value type.
     * @return Pair (Binary value, ownership flag).
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::pair<const BinaryValue*, bool> asBinaryInternal() const;

    /**
     * Returns CLOB stream. Attempts to cast current value type into required value type.
     * @param format Format to be used for conversion to CLOB is some cases. If nullptr,
     *               default format will be used.
     * @return Pair (CLOB stream, ownership flag).
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::pair<const ClobStream*, bool> asClobInternal(const char* format) const;

    /**
     * Returns BLOB stream. Attempts to cast current value type into required value type.
     * @return Pair (BLOB stream, ownership flag).
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::pair<const BlobStream*, bool> asBlobInternal() const;

    /**
     * Converts string to boolean value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    bool stringToBool(const std::string& s) const;

    /**
     * Converts string to 8-bit signed integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int8_t stringToInt8(const std::string& s) const;

    /**
     * Converts string to 8-bit unsigned integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint8_t stringToUInt8(const std::string& s) const;

    /**
     * Converts string to 16-bit signed integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int16_t stringToInt16(const std::string& s) const;

    /**
     * Converts string to 16-bit unsigned integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint16_t stringToUInt16(const std::string& s) const;

    /**
     * Converts string to 32-bit signed integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int32_t stringToInt32(const std::string& s) const;

    /**
     * Converts string to 32-bit unsigned integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint32_t stringToUInt32(const std::string& s) const;

    /**
     * Converts string to 64-bit signed integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int64_t stringToInt64(const std::string& s) const;

    /**
     * Converts string to 64-bit unsigned integer value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint64_t stringToUInt64(const std::string& s) const;

    /**
     * Converts string to 32-bit floating point value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    float stringToFloat(const std::string& s) const;

    /**
     * Converts string to 64-bit floating point value as part of public type conversion function.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    double stringToDouble(const std::string& s) const;

    /**
     * Converts string to binary.
     * @param s A string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    BinaryValue* stringToBinary(const std::string& s) const
    {
        auto data = reinterpret_cast<const std::uint8_t*>(s.c_str());
        return new BinaryValue(data, data + s.length());
    }

    /**
     * Converts string to date/time.
     * @param s A string.
     * @param len String length
     * @param format A date/time format.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime stringToDateTime(const char* s, std::size_t len, const char* format) const;

    /**
     * Converts string to date/time.
     * @param s A string.
     * @param format A date/time format.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime stringToDateTime(const std::string& s, const char* format) const
    {
        return stringToDateTime(s.c_str(), s.size(), format);
    }

    /**
     * Converts binary value to boolean value as part of public type conversion function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    bool binaryToBool(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 8-bit signed integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int8_t binaryToInt8(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 8-bit unsigned integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint8_t binaryToUInt8(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 16-bit signed integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int16_t binaryToInt16(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 16-bit unsigned integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint16_t binaryToUInt16(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 32-bit signed integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int32_t binaryToInt32(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 32-bit unsigned integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint32_t binaryToUInt32(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 64-bit signed integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::int64_t binaryToInt64(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 64-bit unsigned integer value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::uint64_t binaryToUInt64(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 32-bit floating point value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    float binaryToFloat(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to 64-bit floating point value as part of public type conversion
     * function.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    double binaryToDouble(const BinaryValue& binaryValue) const;

    /**
     * Converts binary value to string.
     * @param binaryValue A binary value.
     * @param destValueType Destination value type.
     * @param maxOutputLength Maximum length of output string.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::string* binaryToString(const BinaryValue& binaryValue, VariantType destValueType,
            std::size_t maxOutputLength) const;

    /**
     * Converts binary value to date/time.
     * @param binaryValue A binary value.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime binaryToDateTime(const BinaryValue& binaryValue) const;

    /**
     * Converts UNIX timestamp to date/time.
     * @param timestamp A timestamp.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime timestampToDateTime(std::time_t timestamp) const;

    /**
     * Converts struct tm to date/time.
     * @param tm A struct tm.
     * @return Converted value.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    RawDateTime tmToDateTime(const std::tm& tm) const;

    /**
     * Receives CLOB content as string during type conversion.
     * @param destValueType Destination value type.
     * @return CLOB content as string.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    std::string readClobAsString(VariantType destValueType) const;

    /**
     * Receives BLOB content as binary value during type conversion.
     * @param destValueType Destination value type.
     * @return BLOB content as binary data buffer.
     * @throw VariantTypeCastError if value type cast is not possible.
     */
    BinaryValue readBlobAsBinary(VariantType destValueType) const;

    /**
     * Get a default date format specifier string from passed string length
     * @param strLen A string length.
     * @return Format specifier.
     * @throw std::invalid_argument if @ref strLen is zero.
     */
    static const char* getDateTimeFormat(std::size_t strLen)
    {
        if (SIODB_UNLIKELY(strLen == 0)) throw std::invalid_argument("String length is 0");
        return strLen > RawDateTime::kMaxDateStringLength ? kDefaultDateTimeFormat
                                                          : kDefaultDateFormat;
    }

    /**
     * Get a default date format specifier string for passed string with date
     * @param dateStr A string with a date.
     * @return format specifier.
     * @throw std::invalid_argument if @ref dateStr length is zero.
     */
    static const char* getDateTimeFormat(const std::string& dateStr)
    {
        return getDateTimeFormat(dateStr.size());
    }

private:
    /** A value */
    Value m_value;

    /** Value type */
    VariantType m_valueType;

    /** Null value */
    static const Variant m_nullValue;

    /** Empty string value */
    static const Variant m_emptyStringValue;

    /** Hex conversion table */
    static const char m_hexConversionTable[16];

    friend std::ostream& operator<<(std::ostream& os, const Variant& v);
};

/**
 * Stream output operator for the @ref Variant.
 * @param os Output stream.
 * @param v A value.
 */
std::ostream& operator<<(std::ostream& os, const Variant& v);

}  // namespace siodb::iomgr::dbengine
