// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Index.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "IndexColumn.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FsUtils.h>

namespace siodb::iomgr::dbengine {

Index::Index(Table& table, IndexType type, const std::string& name, const IndexKeyTraits& keyTraits,
        std::size_t valueSize, KeyCompareFunction keyCompare, bool unique,
        const IndexColumnSpecificationList& columns)
    : m_table(table)
    , m_type(type)
    , m_name(name)
    , m_id(m_table.getDatabase().generateNextIndexId(m_table.isSystemTable()))
    , m_dataDir(ensureDataDir(
              utils::constructPath(table.getDataDir(), kIndexDataDirPrefix, m_id), true))
    , m_keySize(keyTraits.getKeySize())
    , m_valueSize(valueSize)
    , m_kvPairSize(m_keySize + m_valueSize)
    , m_keyCompare(keyCompare)
    , m_unique(unique)
    , m_columns(makeIndexColumns(columns))
{
}

Index::Index(Table& table, const IndexRecord& indexRecord, const IndexKeyTraits& keyTraits,
        std::size_t valueSize, KeyCompareFunction keyCompare)
    : m_table(validateTable(table, indexRecord))
    , m_type(indexRecord.m_type)
    , m_name(indexRecord.m_name)
    , m_id(indexRecord.m_id)
    , m_dataDir(ensureDataDir(
              utils::constructPath(table.getDataDir(), kIndexDataDirPrefix, m_id), false))
    , m_keySize(keyTraits.getKeySize())
    , m_valueSize(valueSize)
    , m_kvPairSize(m_keySize + m_valueSize)
    , m_keyCompare(keyCompare)
    , m_unique(indexRecord.m_unique)
    , m_columns(makeIndexColumns(indexRecord.m_columns))
{
}

std::string Index::getDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_table.getDatabaseName() << "'.'" << m_table.getName() << "'.'" << m_name
        << '\'';
    return oss.str();
}

std::string Index::getDisplayCode() const
{
    std::ostringstream oss;
    oss << m_table.getDatabaseUuid() << '.' << m_table.getId() << '.' << m_id;
    return oss.str();
}

std::string Index::makeIndexFilePath(std::uint64_t fileId) const
{
    return utils::constructPath(m_dataDir, kIndexFilePrefix, fileId, kDataFileExtension);
}

// --------- internal -----------

void Index::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexInitializationFlagFile,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, "create file failed");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << std::time(nullptr) << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexInitializationFlagFile,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, "write failed");
    }
}

Table& Index::validateTable(Table& table, const IndexRecord& indexRecord)
{
    if (indexRecord.m_tableId == table.getId()) return table;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexTable, indexRecord.m_id,
            indexRecord.m_tableId, table.getDatabaseName(), table.getName(),
            table.getDatabaseUuid(), table.getId());
}

std::string&& Index::ensureDataDir(std::string&& dataDir, bool create) const
{
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const auto initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        if (initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorIndexAlreadyExists,
                    m_table.getDatabaseName(), m_name);
        }
        try {
            const fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexDataDir, dataDir,
                    m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                    m_table.getId(), m_id, ex.code().value(), ex.code().message());
        }
    } else {
        if (!boost::filesystem::exists(dataDir))
            throwDatabaseError(IOManagerMessageId::kErrorIndexDataFolderDoesNotExist,
                    m_table.getDatabaseName(), m_table.getName(), m_name, dataDir);

        if (!initFlagFileExists)
            throwDatabaseError(IOManagerMessageId::kErrorIndexInitFileDoesNotExist,
                    m_table.getDatabaseName(), m_table.getName(), m_name, initFlagFile);
    }
    return std::move(dataDir);
}

Index::IndexColumnCollection Index::makeIndexColumns(
        const IndexColumnSpecificationList& indexColumnSpecs) const
{
    IndexColumnCollection result;
    result.reserve(indexColumnSpecs.size());
    for (const auto& indexColumnSpec : indexColumnSpecs) {
        result.push_back(std::make_shared<IndexColumn>(stdext::as_mutable(*this),
                indexColumnSpec.m_columnDefinition, indexColumnSpec.m_isSortDescending));
    }
    return result;
}

Index::IndexColumnCollection Index::makeIndexColumns(
        const IndexColumnRegistry& indexColumnRegistry) const
{
    IndexColumnCollection result;
    result.reserve(indexColumnRegistry.size());
    for (const auto& indexColumnRecord : indexColumnRegistry.byId()) {
        result.push_back(
                std::make_shared<IndexColumn>(stdext::as_mutable(*this), indexColumnRecord));
    }
    return result;
}

}  // namespace siodb::iomgr::dbengine
