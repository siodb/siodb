// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDefinition.h"
#include "ColumnDefinitionConstraintList.h"
#include "Index.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// STL headers
#include <fstream>

// Boost headers
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/adaptors.hpp>

// Protobuf messages
#include <siodb/common/proto/SystemTables.pb.h>

namespace siodb::iomgr::dbengine {

namespace {

/** System object serialiation helper class. */
class SystemObjectSerializer {
public:
    /** Maximum serialized object size */
    static constexpr std::size_t kMaxObjectSerializedSize = 0x10000;

public:
    /** 
     * Initializes object of class SystemObjectSerializer.
     * @param database Database object.
     * @param file Destination file object.
     * @param initialFilePos Initial file position.
     */
    SystemObjectSerializer(const Database& database, io::File& file, off_t initialFilePos = 0)
        : m_database(database)
        , m_file(file)
        , m_filePos(initialFilePos)
        , m_buffer(512)
    {
    }

    DECLARE_NONCOPYABLE(SystemObjectSerializer);

    /**
     * Serializes system objects from a given collection into file.
     * @tparam Collection Object collection type.
     * @param objectTypeName Object type name.
     * @param objects Collection of objects.
     * @param firstUserObjectId First user-space object ID.
     * @throw std::runtime_error if serializarion fails.
     */
    template<class Collection>
    void serialize(
            const char* objectTypeName, const Collection& objects, unsigned firstUserObjectId);

    /**
     * Deserializes system objects from a given collection into file.
     * @tparam Collection Object collection type.
     * @param objectTypeName Object type name.
     * @param objects Collection of objects.
     * @throw std::runtime_error if serializarion fails.
     */
    template<class Collection>
    void deserialize(const char* objectTypeName, Collection& objects);

private:
    /** Database to which I/O operations belong */
    const Database& m_database;

    /** File on which I/O operations are performed */
    io::File& m_file;

    /** Current position in the file */
    off_t m_filePos;

    /** I/O buffer */
    BinaryValue m_buffer;
};

template<class Collection>
void SystemObjectSerializer::serialize(
        const char* objectTypeName, const Collection& objects, unsigned firstUserObjectId)
{
    const auto& index = objects.byId();

    // Count objects
    const auto objectCount = std::count_if(
            index.cbegin(), index.cend(), [firstUserObjectId](const auto& object) noexcept {
                return object.m_id < firstUserObjectId;
            });

    // Write object count
    ::pbeEncodeUInt32(static_cast<std::uint32_t>(objectCount), m_buffer.data());
    m_file.writeChecked(m_buffer.data(), 4, m_filePos);
    m_filePos += 4;

    // Exit at this point if there are no objects
    if (objectCount == 0) return;

    // Write objects
    for (const auto& obj : index) {
        // Skip non-system objects
        if (obj.m_id >= firstUserObjectId) continue;

        // Serialize object size
        const auto serializedSize = obj.getSerializedSize() + 4;
        if (m_buffer.size() < serializedSize) m_buffer.resize(serializedSize);
        ::pbeEncodeUInt32(static_cast<std::uint32_t>(serializedSize - 4), m_buffer.data());

        // Serialize object
        const auto p = obj.serializeUnchecked(m_buffer.data() + 4);
        if (p != m_buffer.data() + serializedSize) {
            std::ostringstream err;
            err << "object type '" << objectTypeName << "' id=" << obj.m_id
                << ": expected serialized size " << (serializedSize - 4) << " bytes, but got "
                << ((p - m_buffer.data()) - 4) << " bytes actually. Memory may be corrupted.";
            throwDatabaseError(IOManagerMessageId::kErrorCannotSerializeSystemObject,
                    m_database.getName(), m_database.getUuid(), err.str());
        }

        // Write object data
        try {
            m_file.writeChecked(m_buffer.data(), serializedSize, m_filePos);
            m_filePos += serializedSize;
        } catch (FileWriteError& ex) {
            std::ostringstream err;
            err << "object type '" << objectTypeName << "' id=" << obj.m_id << ": "
                << ex.getErrorCode() << ' ' << ex.what();
            throwDatabaseError(IOManagerMessageId::kErrorCannotWriteSystemObjectsFile,
                    m_database.getName(), m_database.getUuid(), err.str());
        }
    }
}

template<class Collection>
void SystemObjectSerializer::deserialize(const char* objectTypeName, Collection& objects)
{
    // Clear container
    objects.clear();

    // Read object count
    std::uint32_t objectCount = 0;
    m_file.readChecked(reinterpret_cast<std::uint8_t*>(&objectCount), 4, m_filePos);
    m_filePos += 4;

    // Read objects
    for (std::uint32_t i = 0; i < objectCount; ++i) {
        std::uint32_t objectSize = 0;
        try {
            // Read object size
            m_file.readChecked(
                    reinterpret_cast<std::uint8_t*>(&objectSize), sizeof(objectSize), m_filePos);
            m_filePos += 4;
            if (objectSize > kMaxObjectSerializedSize) {
                std::ostringstream err;
                err << "object type '" << objectTypeName << "' #" << (i + 1) << " of "
                    << objectCount << ": size is too big: " << objectSize;
                throwDatabaseError(IOManagerMessageId::kErrorCannotReadSystemObjectsFile,
                        m_database.getName(), m_database.getUuid(), err.str());
            }

            // Prepare buffer
            if (m_buffer.size() < objectSize) m_buffer.resize(objectSize);

            // Read object
            m_file.readChecked(m_buffer.data(), objectSize, m_filePos);
            m_filePos += objectSize;
        } catch (FileReadError& ex) {
            std::ostringstream err;
            err << "object type '" << objectTypeName << "' #" << (i + 1) << " of " << objectCount
                << ": " << ex.getErrorCode() << ' ' << ex.what();
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadSystemObjectsFile,
                    m_database.getName(), m_database.getUuid(), err.str());
        }

        // Deserialize object
        typename Collection::value_type r;
        try {
            r.deserialize(m_buffer.data(), objectSize);
        } catch (std::exception& ex) {
            std::ostringstream err;
            err << "object type '" << objectTypeName << "' #" << (i + 1) << " of " << objectCount
                << ": " << ex.what();
            throwDatabaseError(IOManagerMessageId::kErrorCannotDeserializeSystemObject,
                    m_database.getName(), m_database.getUuid(), err.str());
        }

        // Save object into collection
        objects.insert(std::move(r));
    }

    // Check number of objects
    if (objects.size() != objectCount) {
        std::ostringstream err;
        err << "object type '" << objectTypeName << "': expected " << objectCount
            << " objects, but actually got " << objects.size();
        throwDatabaseError(IOManagerMessageId::kErrorCannotDeserializeSystemObject,
                m_database.getName(), m_database.getUuid(), err.str());
    }
}

}  // namespace

void Database::loadSystemObjectsInfo()
{
    io::FilePtr f;
    try {
        f = openFile(makeSystemObjectsFilePath());
    } catch (std::exception& ex) {
        throwDatabaseError(
                IOManagerMessageId::kErrorCannotOpenSystemObjectsFile, m_name, m_uuid, ex.what());
    }

    SystemObjectSerializer s(*this, *f);
    s.deserialize("Table", m_tableRegistry);
    s.deserialize("ColumnSet", m_columnSetRegistry);
    s.deserialize("Column", m_columnRegistry);
    s.deserialize("ColumnDefinition", m_columnDefinitionRegistry);
    s.deserialize("Constraint", m_constraintRegistry);
    s.deserialize("ConstraintDefinition", m_constraintDefinitionRegistry);
    s.deserialize("Index", m_indexRegistry);
}

void Database::saveSystemObjectsInfo() const
{
    // Open file
    const auto filePath = makeSystemObjectsFilePath();
    const auto tmpFilePath = filePath + ".tmp";
    io::FilePtr f;
    try {
        f = createFile(tmpFilePath, O_DSYNC, 0660);
    } catch (std::exception& ex) {
        throwDatabaseError(
                IOManagerMessageId::kErrorCannotWriteSystemObjectsFile, m_name, m_uuid, ex.what());
    }

    SystemObjectSerializer s(*this, *f);
    s.serialize("Table", m_tableRegistry, kFirstUserTableId);
    s.serialize("ColumnSet", m_columnSetRegistry, kFirstUserTableColumnSetId);
    s.serialize("Column", m_columnRegistry, kFirstUserTableColumnId);
    s.serialize("ColumnDefinition", m_columnDefinitionRegistry, kFirstUserTableColumnDefinitionId);
    s.serialize("Constraint", m_constraintRegistry, kFirstUserTableConstraintId);
    s.serialize("ConstraintDefinition", m_constraintDefinitionRegistry,
            kFirstUserTableConstraintDefinitionId);
    s.serialize("Index", m_indexRegistry, kFirstUserTableIndexId);

    // Close file
    f.reset();

    // Move temporary file into permanent
    boost::system::error_code ec;
    fs::rename(tmpFilePath, filePath, ec);
    if (ec) {
        std::ostringstream err;
        err << ec.value() << ' ' << ec.message();
        throwDatabaseError(
                IOManagerMessageId::kErrorCannotMoveSystemObjectsFile, m_name, m_uuid, err.str());
    }
}

}  // namespace siodb::iomgr::dbengine
