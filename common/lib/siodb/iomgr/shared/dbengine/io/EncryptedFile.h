// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "File.h"
#include "../crypto/ciphers/CipherContext.h"

// Common project headers
#include <siodb/common/utils/Align.h>

namespace siodb::iomgr::dbengine::io {

/** Provides encrypted binary file I/O. */
class EncryptedFile : public File {
public:
    DECLARE_NONCOPYABLE(EncryptedFile);

    /**
     * Initializes object of class EncryptedFile. Creates new file.
     * @param path File path.
     * @param extraFlags Additional open flags.
     * @param createMode File creation mode.
     * @param encryptionContext Encryption context.
     * @param decryptionContext Decryption context.
     * @param initialSize Initial plaintext size of the file.
     * @throw std::system_error if creating new file fails
     */
    EncryptedFile(const std::string& path, int flags, int createMode,
            const crypto::ConstCipherContextPtr& encryptionContext,
            const crypto::ConstCipherContextPtr& decryptionContext, off_t initialSize = 0);

    /**
     * Initializes object of class EncryptedFile. Open existing file.
     * @param path File path.
     * @param extraFlags Additional create flags.
     * @param encryptionContext Encryption context.
     * @param decryptionContext Decryption context.
     * @throw std::system_error if opening existing file fails.
     */
    EncryptedFile(const std::string& path, int flags,
            const crypto::ConstCipherContextPtr& encryptionContext,
            const crypto::ConstCipherContextPtr& decryptionContext);

    /**
     * Returns block size.
     * @return Block size.
     */
    auto getBlockSize() const noexcept
    {
        return m_blockSize;
    }

    /**
     * Reads specified amount of data from file starting at a given offset.
     * If pread() system call succeeds but reads less then specified, next attempts are taken
     * to read subsequent portion of data until specified number of bytes is read.
     * @param[out] buffer A buffer for data.
     * @param size Desired data size.
     * @param offset Plaintext offset.
     * @return Number of bytes actually read. Values less than requested indicate error.
     *         In such case, getLastError() will return an error code. If error code is 0,
     *         then end of file is reached.
     */
    std::size_t read(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept override;

    /**
     * Writes specified amount of data to file starting at a give offset.
     * If write() system call succeeds but writes less then specified, next attempts are taken
     * to write subsequent portion of data, until specified number of bytes written.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Plaintext offset.
     * @return Number of bytes known to be written successfully. Value less than requested
     *         indicates error. In such case, getLastError() will return an error code.
     */
    std::size_t write(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept override;

    /**
     * Returns current file size.
     * @return Plaintext size.
     */
    off_t getFileSize() noexcept override;

    /**
     * Provides file statistics.
     * @param st Statistics structure.
     * @return true if operation suceeded, false otherwise.
     */
    bool stat(struct stat& st) noexcept override;

    /**
     * Extends file by a length bytes.
     * @param length Number of bytes to allocate.
     * @return true if operation suceeded, false otherwise. In the case of failure,
     *         getLastError() will return an error code.
     */
    bool extend(off_t length) noexcept override;

private:
    /**
     * Reads specified amount of data from file starting at a given offset.
     * If pread() system call succeeds but reads less then specified, next attempts are taken
     * to read subsequent portion of data until specified number of bytes is read.
     * @param buffer A buffer for data.
     * @param size Desired data size.
     * @param offset Starting offset.
     * @return Number of bytes actually read. Values less than requested indicate error.
     *         In such case, m_lastError an error code. Error code 0 indicates that end of file
     *         is reached.
     */
    std::size_t readInternal(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept;

    /**
     * Writes specified amount of data to file starting at a give offset.
     * If write() system call succeeds but writes less then specified, next attempts are taken
     * to write subsequent portion of data, until specified number of bytes written.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @return Number of bytes known to be written successfully. Value less than requested
     *         indicates error. In such case m_lastError will contain an error code.
     */
    std::size_t writeInternal(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept;

    /**
     * Updates existing block in the file.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @return true if block was updated succesfully, false otherwise. In the case of failure,
     *         m_lastError will contain an error code.
     */
    bool updateBlock(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept;

    /**
     * Writes block beyond the end file.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @return true if block was updated succesfully, false otherwise. In the case of failure,
     *         m_lastError will contain an error code.
     */
    bool writeBlock(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept;

    /**
     * Reads header in the beginning of the file.
     * @return true if opoeration succeeded, false otherwise. In the case of failure,
     *         m_lastError will contain an error code.
     */
    bool readHeader() noexcept;

    /**
     * Writes header in the beginning of the file.
     * @return true if opoeration succeeded, false otherwise.
     *         If failed, m_lastError will contain an error code.
     */
    bool writeHeader() noexcept;

private:
    /**
     * Returns EOF offset.
     * @return EOF offset.
     */
    off_t getEofOffset() const noexcept
    {
        return utils::alignUp(m_plaintextSize, m_blockSize) + m_headerBuffer.size();
    }

private:
    /** Plaintext size */
    off_t m_plaintextSize;

    /** Context for encryption operations */
    const crypto::ConstCipherContextPtr m_encryptionContext;

    /** Context for decryption operations */
    const crypto::ConstCipherContextPtr m_decryptionContext;

    /** Block size */
    const std::size_t m_blockSize;

    /** Header buffer */
    BinaryValue m_headerBuffer;

    /** I/O buffer blocks count */
    const std::size_t m_headerBufferBlockCount;

    /** I/O buffer */
    BinaryValue m_dataBuffer;

    /** I/O buffer blocks count */
    const std::size_t m_dataBufferBlockCount;

    /** Useful size of I/O buffer, which can store number of full blocks */
    const std::size_t m_dataBufferUsefulSize;

    /** Header plaintext size */
    static constexpr std::size_t kHeaderPlaintextSize = sizeof(std::uint64_t);

    /** I/O buffer size */
    static constexpr std::size_t kDataBufferSize = 8192;
};

}  // namespace siodb::iomgr::dbengine::io
