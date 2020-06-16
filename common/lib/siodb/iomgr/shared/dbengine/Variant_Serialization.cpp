// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// Project headers
#include "lob/BinaryValueBlobStream.h"
#include "lob/StringClobStream.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t Variant::getSerializedSize() const noexcept
{
    switch (m_valueType) {
        case VariantType::kBool:
        case VariantType::kInt8:
        case VariantType::kUInt8: return 2;
        case VariantType::kInt16: return ::getVarIntSize(m_value.m_i16) + 1;
        case VariantType::kUInt16: return ::getVarIntSize(m_value.m_ui16) + 1;
        case VariantType::kInt32: return ::getVarIntSize(m_value.m_i32) + 1;
        case VariantType::kUInt32: return ::getVarIntSize(m_value.m_ui32) + 1;
        case VariantType::kInt64: return ::getVarIntSize(m_value.m_i64) + 1;
        case VariantType::kUInt64: return ::getVarIntSize(m_value.m_ui64) + 1;
        case VariantType::kFloat: return 5;
        case VariantType::kDouble: return 9;
        case VariantType::kDateTime: return m_value.m_dt->getSerializedSize() + 1;
        case VariantType::kString: return ::getSerializedSize(*m_value.m_string) + 1;
        case VariantType::kBinary: return ::getSerializedSize(*m_value.m_binary) + 1;
        case VariantType::kClob: {
            // Do not throw exception here, it will be thrown during serialization.
            // Return value as if it was zero-size CLOB.
            if (SIODB_UNLIKELY(m_value.m_clob->getSize() > kMaxStringValueLength)) return 2;
            return static_cast<std::size_t>(::getVarIntSize(m_value.m_clob->getSize()))
                   + m_value.m_clob->getSize() + 1;
        }
        case VariantType::kBlob: {
            // Do not throw exception here, it will be thrown during serialization.
            // Return value as if it was zero-size BLOB.
            if (SIODB_UNLIKELY(m_value.m_blob->getSize() > kMaxBinaryValueLength)) return 2;
            return static_cast<std::size_t>(::getVarIntSize(m_value.m_blob->getSize()))
                   + m_value.m_blob->getSize() + 1;
        }
        default: return 1;  // Data type only
    }
}

std::uint8_t* Variant::serializeUnchecked(std::uint8_t* buffer) const
{
    *buffer++ = static_cast<std::uint8_t>(m_valueType);
    switch (m_valueType) {
        case VariantType::kBool: {
            *buffer = m_value.m_bool ? 1 : 0;
            return buffer + 1;
        }
        case VariantType::kInt8:
        case VariantType::kUInt8: {
            *buffer = m_value.m_ui8;
            return buffer + 1;
        }
        case VariantType::kInt16: return ::encodeVarInt(m_value.m_i16, buffer);
        case VariantType::kUInt16: return ::encodeVarInt(m_value.m_ui16, buffer);
        case VariantType::kInt32: return ::encodeVarInt(m_value.m_i32, buffer);
        case VariantType::kUInt32: return ::encodeVarInt(m_value.m_ui32, buffer);
        case VariantType::kInt64: return ::encodeVarInt(m_value.m_i64, buffer);
        case VariantType::kUInt64: return ::encodeVarInt(m_value.m_ui64, buffer);
        case VariantType::kFloat: return ::pbeEncodeFloat(m_value.m_float, buffer);
        case VariantType::kDouble: return ::pbeEncodeDouble(m_value.m_double, buffer);
        case VariantType::kDateTime: return m_value.m_dt->serialize(buffer);
        case VariantType::kString: {
            buffer = ::encodeVarInt(m_value.m_string->length(), buffer);
            if (!m_value.m_string->empty())
                std::memcpy(buffer, m_value.m_string->c_str(), m_value.m_string->length());
            return buffer + m_value.m_string->length();
        }
        case VariantType::kBinary: {
            buffer = ::encodeVarInt(m_value.m_binary->size(), buffer);
            if (!m_value.m_binary->empty())
                std::memcpy(buffer, m_value.m_binary->data(), m_value.m_binary->size());
            return buffer + m_value.m_binary->size();
        }
        case VariantType::kClob: {
            // Check size
            if (SIODB_UNLIKELY(m_value.m_clob->getSize() > kMaxStringValueLength)) {
                throw VariantSerializationError(
                        "CLOB is too long: " + std::to_string(m_value.m_clob->getSize()));
            }

            // Rewind before
            try {
                m_value.m_clob->rewind();
            } catch (std::exception& ex) {
                throw VariantSerializationError(
                        "Could not rewind CLOB stream before serialization: "
                        + std::string(ex.what()));
            }

            buffer = ::encodeVarInt(m_value.m_clob->getSize(), buffer);
            std::uint32_t remaining = m_value.m_clob->getSize();
            while (remaining > 0) {
                const auto actual = m_value.m_clob->read(buffer, remaining);
                if (actual < 0) return nullptr;
                remaining -= actual;
                buffer += actual;
            }

            // Rewind after
            try {
                m_value.m_clob->rewind();
            } catch (std::exception& ex) {
                throw VariantSerializationError("Could not rewind CLOB stream after serialization: "
                                                + std::string(ex.what()));
            }

            return buffer;
        }
        case VariantType::kBlob: {
            // Check size
            if (SIODB_UNLIKELY(m_value.m_blob->getSize() > kMaxBinaryValueLength)) {
                throw VariantSerializationError(
                        "BLOB is too long: " + std::to_string(m_value.m_blob->getSize()));
            }

            // Rewind before
            try {
                m_value.m_blob->rewind();
            } catch (std::exception& ex) {
                throw VariantSerializationError(
                        "Could not rewind BLOB stream before serialization: "
                        + std::string(ex.what()));
            }

            buffer = ::encodeVarInt(m_value.m_blob->getSize(), buffer);
            std::uint32_t remaining = m_value.m_blob->getSize();
            while (remaining > 0) {
                const auto actual = m_value.m_blob->read(buffer, remaining);
                if (actual < 0) return nullptr;
                remaining -= actual;
                buffer += actual;
            }

            // Rewind after
            try {
                m_value.m_blob->rewind();
            } catch (std::exception& ex) {
                throw VariantSerializationError("Could not rewind BLOB stream after serialization: "
                                                + std::string(ex.what()));
            }

            return buffer;
        }
        default: return buffer;
    }
}

std::size_t Variant::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    if (SIODB_UNLIKELY(length == 0)) throw VariantDeserializationError("Data length is zero");
    const auto b = *buffer++;
    if (SIODB_UNLIKELY(b >= static_cast<std::uint8_t>(VariantType::kMax))) {
        throw VariantDeserializationError(
                "Invalid data type" + std::to_string(static_cast<unsigned>(b)));
    }
    const auto t = static_cast<VariantType>(b);
    switch (t) {
        case VariantType::kNull: {
            m_valueType = t;
            return 1;
        }
        case VariantType::kBool: {
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_value.m_bool = *buffer != 0;
            m_valueType = t;
            return 2;
        }
        case VariantType::kInt8:
        case VariantType::kUInt8: {
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_value.m_ui8 = *buffer;
            m_valueType = t;
            return 2;
        }
        case VariantType::kInt16: {
            std::int16_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt int16 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the int16 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_i16 = v;
            return consumed + 1;
        }
        case VariantType::kUInt16: {
            std::uint16_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt uint16 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the uint16 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_ui16 = v;
            return consumed + 1;
        }
        case VariantType::kInt32: {
            std::int32_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt int32 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the int32 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_i32 = v;
            return consumed + 1;
        }
        case VariantType::kUInt32: {
            std::uint32_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt uint32 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the uint32 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_ui32 = v;
            return consumed + 1;
        }
        case VariantType::kInt64: {
            std::int64_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt int64 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the int64 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_i64 = v;
            return consumed + 1;
        }
        case VariantType::kUInt64: {
            std::uint64_t v = 0;
            const int consumed = ::decodeVarInt(buffer, length, v);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt uint64 value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the uint64 value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            m_value.m_ui64 = v;
            return consumed + 1;
        }
        case VariantType::kFloat: {
            if (SIODB_UNLIKELY(length < 4)) {
                throw VariantDeserializationError(
                        "Not enough data for the float value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            ::pbeDecodeFloat(buffer, &m_value.m_float);
            return 5;
        }
        case VariantType::kDouble: {
            if (SIODB_UNLIKELY(length < 8)) {
                throw VariantDeserializationError(
                        "Not enough data for the double value: " + std::to_string(length));
            }
            if (SIODB_UNLIKELY(m_valueType >= kFirstNonPrimitiveVariantType)) clear();
            m_valueType = t;
            ::pbeDecodeDouble(buffer, &m_value.m_double);
            return 9;
        }
        case VariantType::kDateTime: {
            RawDateTime dt;
            const int consumed = dt.deserialize(buffer, length);
            if (SIODB_UNLIKELY(consumed < 0))
                throw VariantDeserializationError("Corrupt date/time value");
            if (SIODB_UNLIKELY(consumed == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the date/time: " + std::to_string(length));
            }
            *this = std::move(dt);
            return consumed + 1;
        }
        case VariantType::kString: {
            std::string s;
            const auto consumed = ::deserializeObject(buffer, length, s);
            *this = std::move(s);
            return consumed + 1;
        }
        case VariantType::kBinary: {
            BinaryValue v;
            const auto consumed = ::deserializeObject(buffer, length, v);
            *this = std::move(v);
            return consumed + 1;
        }
        case VariantType::kClob: {
            std::string s;
            const auto consumed = ::deserializeObject(buffer, length, s);
            *this = new StringClobStream(std::move(s));
            return consumed + 1;
        }
        case VariantType::kBlob: {
            BinaryValue v;
            const auto consumed = ::deserializeObject(buffer, length, v);
            *this = new BinaryValueBlobStream(std::move(v));
            return consumed + 1;
        }
        default: {
            throw VariantDeserializationError("Deserialization of the value type "
                                              + std::to_string(static_cast<int>(t))
                                              + " is not supported");
        }
    }
}

}  // namespace siodb::iomgr::dbengine
