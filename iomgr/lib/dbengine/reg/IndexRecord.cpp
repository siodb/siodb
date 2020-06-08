// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexRecord.h"

// Project headers
#include "Helpers.h"
#include "../IndexColumn.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid IndexRecord::s_classUuid =
        boost::lexical_cast<Uuid>("f6c807ee-f24a-4398-ae44-d189036c7842");

IndexRecord::IndexRecord(const Index& index)
    : m_id(index.getId())
    , m_type(index.getType())
    , m_tableId(index.getTableId())
    , m_unique(index.isUnique())
    , m_name(index.getName())
    , m_dataFileSize(index.getDataFileSize())
    , m_description(index.getDescription())
{
    for (const auto& indexColumn : index.getColumns())
        m_columns.emplace(*indexColumn);
}

std::size_t IndexRecord::getSerializedSize(unsigned version) const noexcept
{
    std::size_t result = Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
                         + ::getVarIntSize(static_cast<std::uint32_t>(m_type))
                         + ::getVarIntSize(m_tableId) + 1U + ::getSerializedSize(m_name)
                         + ::getVarIntSize(static_cast<std::uint32_t>(m_columns.size()))
                         + ::getVarIntSize(m_dataFileSize) + ::getSerializedSize(m_description);
    for (const auto& column : m_columns.byId())
        result += column.getSerializedSize();
    return result;
}

std::uint8_t* IndexRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_type), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    *buffer++ = m_unique ? 1 : 0;
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_columns.size()), buffer);
    for (const auto& column : m_columns.byId())
        buffer = column.serializeUnchecked(buffer);
    buffer = ::encodeVarInt(m_dataFileSize, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t IndexRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    if (length < Uuid::static_size())
        helpers::reportInvalidOrNotEnoughData(kClassName, "$classUuid", 0);
    if (std::memcmp(s_classUuid.data, buffer, Uuid::static_size()) != 0)
        helpers::reportClassUuidMismatch(kClassName, buffer, s_classUuid.data);

    std::size_t totalConsumed = Uuid::static_size();

    std::uint32_t classVersion = 0;
    int consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, classVersion);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "$classVersion", consumed);
    totalConsumed += consumed;

    if (classVersion > kClassVersion)
        helpers::reportClassVersionMismatch(kClassName, classVersion, kClassVersion);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_id);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "id", consumed);
    totalConsumed += consumed;

    std::uint32_t indexType = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, indexType);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "type", consumed);
    totalConsumed += consumed;
    m_type = static_cast<IndexType>(indexType);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    if (length - 1 < totalConsumed)
        helpers::reportInvalidOrNotEnoughData(kClassName, "unique", consumed);
    m_unique = buffer[totalConsumed++] != 0;

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    std::uint32_t columnCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, columnCount);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "column.size", columnCount);
    totalConsumed += consumed;

    std::uint32_t index = 0;
    try {
        m_columns.clear();
        for (; index < columnCount; ++index) {
            IndexColumnRecord r;
            totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
            m_columns.insert(std::move(r));
        }
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "columns[" << index << ']';
        helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
    }

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_dataFileSize);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "dataFileSize", consumed);
    totalConsumed += consumed;

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "description", ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
