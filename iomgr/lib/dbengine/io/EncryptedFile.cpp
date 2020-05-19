// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "EncryptedFile.h"

// Common project headers
#include <siodb/common/io/FileIO.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cassert>
#include <cerrno>
#include <cstring>

// STL headers
#include <sstream>

// Keep all these DEBUG_TRACEs in the code for a while. To be removed a bit later,
// when we are completely confident that it works correctly with our real data.
#ifdef DEBUG_TRACE
#undef DEBUG_TRACE
#define DEBUG_TRACE(x)
#endif

namespace siodb::iomgr::dbengine::io {

EncryptedFile::EncryptedFile(const std::string& path, int extraFlags, int createMode,
        const crypto::ConstCipherContextPtr& encryptionContext,
        const crypto::ConstCipherContextPtr& decryptionContext, off_t initialSize)
    : File(path, extraFlags, createMode,
            utils::alignUp(initialSize
                                   + utils::alignUp(kHeaderPlaintextSize,
                                           encryptionContext->getBlockSizeInBytes()),
                    encryptionContext->getBlockSizeInBytes()))
    , m_plaintextSize(initialSize)
    , m_encryptionContext(encryptionContext)
    , m_decryptionContext(decryptionContext)
    , m_blockSize(encryptionContext->getBlockSizeInBytes())
    , m_headerBuffer(utils::alignUp(kHeaderPlaintextSize, m_blockSize))
    , m_headerBufferBlockCount(m_headerBuffer.size() / m_blockSize)
    , m_dataBuffer(kDataBufferSize)
    , m_dataBufferBlockCount(m_dataBuffer.size() / m_blockSize)
    , m_dataBufferUsefulSize(m_dataBufferBlockCount * m_blockSize)
{
    if (!writeHeader()) throw std::system_error(m_lastError, std::generic_category());
}

EncryptedFile::EncryptedFile(const std::string& path, int extraFlags,
        const crypto::ConstCipherContextPtr& encryptionContext,
        const crypto::ConstCipherContextPtr& decryptionContext)
    : File(path, extraFlags)
    , m_plaintextSize(0)
    , m_encryptionContext(encryptionContext)
    , m_decryptionContext(decryptionContext)
    , m_blockSize(encryptionContext->getBlockSizeInBytes())
    , m_headerBuffer(utils::alignUp(kHeaderPlaintextSize, m_blockSize))
    , m_headerBufferBlockCount(m_headerBuffer.size() / m_blockSize)
    , m_dataBuffer(kDataBufferSize)
    , m_dataBufferBlockCount(m_dataBuffer.size() / m_blockSize)
    , m_dataBufferUsefulSize(m_dataBufferBlockCount * m_blockSize)
{
    struct stat st;
    if (::fstat(m_fd.getFd(), &st) < 0) {
        const int errorCode = errno;
        throw std::system_error(errorCode, std::generic_category());
    }

    if (st.st_size % m_blockSize != 0)
        throw std::system_error(EINVAL, std::generic_category(), "Invalid file size");
    DEBUG_TRACE("file_size=" << st.st_size);

    if (!readHeader()) throw std::system_error(m_lastError, std::generic_category());
    DEBUG_TRACE("plaintext_size=" << m_plaintextSize);

    const off_t expectedFileSize =
            utils::alignUp(m_plaintextSize, m_blockSize) + m_headerBuffer.size();
    if (expectedFileSize != st.st_size)
        throw std::system_error(EINVAL, std::generic_category(), "Invalid data size");
}

std::size_t EncryptedFile::read(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    DEBUG_TRACE("EncryptedFile::READ: buffer=" << VOID_PTR(buffer) << " size=" << size
                                               << " offset=" << offset);

    if (offset < 0) {
        m_lastError = EINVAL;
        return 0;
    }

    return readInternal(buffer, size, offset + m_headerBuffer.size());
}

std::size_t EncryptedFile::write(
        const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    DEBUG_TRACE("EncryptedFile::WRITE: buffer=" << VOID_PTR(buffer) << " size=" << size
                                                << " offset=" << offset);

    if (offset < 0) {
        m_lastError = EINVAL;
        return 0;
    }

    return writeInternal(buffer, size, offset + m_headerBuffer.size());
}

off_t EncryptedFile::getFileSize() noexcept
{
    return m_plaintextSize;
}

bool EncryptedFile::stat(struct stat& st) noexcept
{
    if (::fstat(m_fd.getFd(), &st) < 0) {
        m_lastError = errno;
        return false;
    }
    st.st_size = m_plaintextSize;
    return true;
}

bool EncryptedFile::extend(off_t length) noexcept
{
    DEBUG_TRACE("EncryptedFile::EXTEND: length=" << length);

    if (length < 0) {
        m_lastError = EINVAL;
        return false;
    }

    const auto remainingPlaintextSize =
            m_plaintextSize - utils::alignDown(m_plaintextSize, m_blockSize);
    if (length <= remainingPlaintextSize) {
        m_plaintextSize += length;
        return writeHeader();
    }

    if (::posixFileAllocateExact(m_fd.getFd(), utils::alignUp(m_plaintextSize, m_blockSize),
                length - remainingPlaintextSize)
            == 0) {
        m_plaintextSize += length;
        return writeHeader();
    }

    m_lastError = errno;
    return false;
}

// ----- internals -----

std::size_t EncryptedFile::readInternal(
        std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    DEBUG_TRACE("EncryptedFile::readInternal: buffer=" << VOID_PTR(buffer) << " size=" << size
                                                       << " offset=" << offset);

    std::size_t totalBytesRead = 0;
    const auto alignedDownOffset = utils::alignDown(offset, m_blockSize);
    const auto offsetDiff = offset - alignedDownOffset;

    if (offsetDiff > 0) {
        // Read partial amount of data from the first block

        if (::preadExact(m_fd.getFd(), m_dataBuffer.data(), m_blockSize, alignedDownOffset,
                    kIgnoreSignals)
                != m_blockSize) {
            m_lastError = errno;
            return 0;
        }

        m_decryptionContext->transform(m_dataBuffer.data(), 1, m_dataBuffer.data());

        const auto partialBytes = std::min(m_blockSize - offsetDiff, size);
        std::memcpy(buffer, m_dataBuffer.data() + offsetDiff, partialBytes);

        if (size == partialBytes) return partialBytes;

        offset += partialBytes;
        buffer += partialBytes;
        size -= partialBytes;
        totalBytesRead = partialBytes;
    }

    const auto alignedDownSize = utils::alignDown(size, m_blockSize);
    if (alignedDownSize > 0) {
        // Read data blocks in the middle

        const auto bytesRead =
                ::preadExact(m_fd.getFd(), buffer, alignedDownSize, offset, kIgnoreSignals);

        if (bytesRead != alignedDownSize) {
            m_lastError = errno;
            const auto decryptedBytes = utils::alignDown(bytesRead, m_blockSize);
            if (decryptedBytes > 0)
                m_decryptionContext->transform(buffer, decryptedBytes / m_blockSize, buffer);
            return totalBytesRead + decryptedBytes;
        }

        m_decryptionContext->transform(buffer, alignedDownSize / m_blockSize, buffer);

        offset += alignedDownSize;
        buffer += alignedDownSize;
        size -= alignedDownSize;
        totalBytesRead += alignedDownSize;
    }

    if (size > 0) {
        // Read part of the last block if applicable

        if (::preadExact(m_fd.getFd(), m_dataBuffer.data(), m_blockSize, offset, kIgnoreSignals)
                != m_blockSize) {
            m_lastError = errno;
            return totalBytesRead;
        }

        m_decryptionContext->transform(m_dataBuffer.data(), 1, m_dataBuffer.data());
        std::memcpy(buffer, m_dataBuffer.data(), size);
        totalBytesRead += size;
    }

    return totalBytesRead;
}

std::size_t EncryptedFile::writeInternal(
        const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    DEBUG_TRACE("EncryptedFile::writeInternal: buffer=" << VOID_PTR(buffer) << " size=" << size
                                                        << " offset=" << offset);

    std::size_t totalBytesWritten = 0;

    DEBUG_TRACE("EncryptedFile::writeInternal: size(0)=" << size << " offset=" << offset);
    DEBUG_TRACE("EncryptedFile::writeInternal: totalBytesWritten(0)=" << totalBytesWritten);

    const auto alignedDownOffset = utils::alignDown(offset, m_blockSize);
    const auto offsetDiff = offset - alignedDownOffset;

    if (offsetDiff > 0) {
        // Update partial amount of data in the first block
        const auto bytesToUpdate = std::min(m_blockSize - offsetDiff, size);
        DEBUG_TRACE("EncryptedFile::writeInternal: "
                    << (offset >= getEofOffset() ? "write1=" : "update1=") << bytesToUpdate);
        const bool needToAddBlock = offset >= getEofOffset();
        const bool result = needToAddBlock ? writeBlock(buffer, bytesToUpdate, offset)
                                           : updateBlock(buffer, bytesToUpdate, offset);
        if (!result) return 0;

        const off_t n = offset + bytesToUpdate - m_headerBuffer.size();
        if (needToAddBlock || n > m_plaintextSize) {
            m_plaintextSize = n;
            if (!writeHeader()) return 0;
        }

        offset += bytesToUpdate;
        buffer += bytesToUpdate;
        size -= bytesToUpdate;
        totalBytesWritten = bytesToUpdate;
    }

    DEBUG_TRACE("EncryptedFile::writeInternal: size(1)=" << size << " offset=" << offset);
    DEBUG_TRACE("EncryptedFile::writeInternal: totalBytesWritten(1)=" << totalBytesWritten);

    // Write data blocks in the middle
    auto alignedDownSize = utils::alignDown(size, m_blockSize);
    while (alignedDownSize > 0) {
        const auto bytesToWrite = std::min(m_dataBufferUsefulSize, alignedDownSize);
        DEBUG_TRACE("EncryptedFile::writeInternal: write2=" << bytesToWrite);
        m_encryptionContext->transform(buffer, bytesToWrite / m_blockSize, m_dataBuffer.data());

        const auto bytesWritten = utils::alignDown(::pwriteExact(m_fd.getFd(), m_dataBuffer.data(),
                                                           bytesToWrite, offset, kIgnoreSignals),
                m_blockSize);

        const bool errorOccurred = bytesWritten != bytesToWrite;
        if (errorOccurred) m_lastError = errno;

        totalBytesWritten += bytesWritten;

        const off_t n = offset + bytesWritten - m_headerBuffer.size();
        if (m_plaintextSize < n) {
            m_plaintextSize = n;
            if (!writeHeader()) return totalBytesWritten;
        }

        if (errorOccurred) return totalBytesWritten;

        offset += bytesToWrite;
        buffer += bytesToWrite;
        size -= bytesToWrite;
        alignedDownSize -= bytesToWrite;
    }

    DEBUG_TRACE("EncryptedFile::writeInternal: size(2)=" << size << " offset=" << offset);
    DEBUG_TRACE("EncryptedFile::writeInternal: totalBytesWritten(2)=" << totalBytesWritten);

    if (size > 0) {
        const auto eofOffset = getEofOffset();
        const auto append = offset == getEofOffset();
        DEBUG_TRACE("EncryptedFile::writeInternal: " << (append ? "append3=" : "update3=") << size);
        const bool result =
                append ? writeBlock(buffer, size, eofOffset) : updateBlock(buffer, size, offset);
        if (result) {
            if (append) {
                m_plaintextSize += size;
                if (writeHeader()) totalBytesWritten += size;
            } else
                totalBytesWritten += size;
#ifdef _DEBUG
            size = 0;
#endif
        }
    }

    DEBUG_TRACE("EncryptedFile::writeInternal: size(3)=" << size);
    DEBUG_TRACE("EncryptedFile::writeInternal: totalBytesWritten(3)=" << totalBytesWritten);

#ifdef _DEBUG
    struct stat st;
    if (::fstat(m_fd.getFd(), &st) == 0) {
        DEBUG_TRACE("EncryptedFile::writeInternal: FINALLY: ciphertext_size="
                    << (st.st_size - m_headerBuffer.size()) << " plaintext_size=" << m_plaintextSize
                    << '\n');
    } else {
        DEBUG_TRACE("EncryptedFile::writeInternal: failed to stat: error " << m_lastError);
    }
#endif

    return totalBytesWritten;
}

bool EncryptedFile::updateBlock(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    assert(size > 0 && size <= m_blockSize);
    assert(offset >= 0 && offset <= static_cast<off_t>(getEofOffset() - size));

    const auto blockOffset = utils::alignDown(offset, m_blockSize);
    if (::preadExact(m_fd.getFd(), m_dataBuffer.data(), m_blockSize, blockOffset, kIgnoreSignals)
            != m_blockSize) {
        m_lastError = errno;
        DEBUG_TRACE("updateBlock: READ block failed: at " << blockOffset << ": " << m_lastError
                                                          << " " << std::strerror(m_lastError));
        return false;
    }
    DEBUG_TRACE("updateBlock: read block at " << blockOffset);

    m_decryptionContext->transform(m_dataBuffer.data(), 1, m_dataBuffer.data());
    std::memcpy(m_dataBuffer.data() + (offset - blockOffset), buffer, size);
    m_encryptionContext->transform(m_dataBuffer.data(), 1, m_dataBuffer.data());

    if (::pwriteExact(m_fd.getFd(), m_dataBuffer.data(), m_blockSize, blockOffset, kIgnoreSignals)
            != m_blockSize) {
        m_lastError = errno;
        DEBUG_TRACE("updateBlock: WRITE block failed: at " << blockOffset << ": " << m_lastError
                                                           << " " << std::strerror(m_lastError));
        return false;
    }

    DEBUG_TRACE("updateBlock: updated block at " << blockOffset);
    return true;
}

bool EncryptedFile::writeBlock(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    assert(size > 0 && size <= m_blockSize);
    assert(offset >= 0 && offset >= getEofOffset());

    const auto leadingGapSize = offset - utils::alignDown(offset, m_blockSize);
    if (leadingGapSize > 0) std::memset(m_dataBuffer.data(), 0, leadingGapSize);
    std::memcpy(m_dataBuffer.data() + leadingGapSize, buffer, size);
    const auto endOfData = leadingGapSize + size;
    const auto trailingGapSize = m_blockSize - endOfData;
    if (trailingGapSize > 0) std::memset(m_dataBuffer.data() + endOfData, 0, trailingGapSize);
    m_encryptionContext->transform(m_dataBuffer.data(), 1, m_dataBuffer.data());

    const auto blockOffset = utils::alignUp(offset, m_blockSize);
    if (::pwriteExact(m_fd.getFd(), m_dataBuffer.data(), m_blockSize, blockOffset, kIgnoreSignals)
            != m_blockSize) {
        m_lastError = errno;
        return false;
    }

    DEBUG_TRACE("writeBlock: added block at " << blockOffset);
    return true;
}

bool EncryptedFile::readHeader() noexcept
{
    union {
        std::uint8_t m_bytes[8];
        std::int64_t m_value;
    } v;
    if (readInternal(v.m_bytes, sizeof(v.m_bytes), 0) == sizeof(v.m_bytes)) {
        ::pbeDecodeInt64(v.m_bytes, &v.m_value);
        m_plaintextSize = v.m_value;
        return true;
    }
    return false;
}

bool EncryptedFile::writeHeader() noexcept
{
    ::pbeEncodeInt64(m_plaintextSize, m_headerBuffer.data());
    m_encryptionContext->transform(
            m_headerBuffer.data(), m_headerBuffer.size() / m_blockSize, m_headerBuffer.data());
    if (::pwriteExact(m_fd.getFd(), m_headerBuffer.data(), m_headerBuffer.size(), 0, kIgnoreSignals)
            == m_headerBuffer.size())
        return true;
    m_lastError = errno;
    return false;
}

}  // namespace siodb::iomgr::dbengine::io
