// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/ConstraintType.h>

namespace siodb::iomgr::dbengine {

class ConstraintDefinition;

/** In-memory constraint definition registry record */
struct ConstraintDefinitionRecord {
    /** Initializes object of class ConstraintDefinitionRecord. */
    ConstraintDefinitionRecord() noexcept
        : m_id(0)
        , m_type(ConstraintType::kNotNull)
        , m_hash(computeHash())
    {
    }

    /**
     * Initializes object of class ConstraintDefinitionRecord.
     * @param id Constraint ID.
     * @param type Constraint type.
     * @param expression Constraint expression.
     */
    ConstraintDefinitionRecord(
            std::uint64_t id, ConstraintType type, BinaryValue&& expression) noexcept
        : m_id(id)
        , m_type(type)
        , m_expression(std::move(expression))
        , m_hash(computeHash())
    {
    }

    /**
     * Initializes object of class ConstraintDefinitionRecord.
     * @param constraintDefinition Constraint definition.
     */
    explicit ConstraintDefinitionRecord(const ConstraintDefinition& constraintDefinition);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ConstraintDefinitionRecord& other) const noexcept
    {
        return m_id == other.m_id && m_type == other.m_type && m_expression == other.m_expression
               && m_hash == other.m_hash;
    }

    /**
     * Computes hash of this constraint definition.
     * @return Hash value.
     */
    std::uint64_t computeHash() const noexcept
    {
        return computeHash(m_type, m_expression);
    }

    /**
     * Computes hash of the constraint definition.
     * @param constraintType Constraint type.
     * @param expression Constraint expression.
     * @return Hash value.
     */
    static std::uint64_t computeHash(
            ConstraintType constraintType, const BinaryValue& binaryValue) noexcept;

    /**
     * Indicates whether other record constains same constaint definition.
     * @param other Other record.
     * @return true if this and other record constain the same constraint definition,
     *         false otherwise.
     */
    bool isEqualDefinition(const ConstraintDefinitionRecord& other) const noexcept
    {
        return m_type == other.m_type && m_expression == other.m_expression;
    }

    /**
     * Returns buffer size required to serialize this object.
     * @param version Target version.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize(unsigned version = kClassVersion) const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @param version Target version.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer, unsigned version = kClassVersion) const
            noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Constraint definition ID */
    std::uint64_t m_id;

    /** Constraint type */
    ConstraintType m_type;

    /** Constraint expression encoded in the binary format. */
    BinaryValue m_expression;

    /** Constraint definition hash */
    std::uint64_t m_hash;

    /** Hash seed. sqrt(2.0) as uint64. */
    constexpr static std::uint64_t kHashSeed = 0x3ff6a09e667f3bcd;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ConstraintDefinitionRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
