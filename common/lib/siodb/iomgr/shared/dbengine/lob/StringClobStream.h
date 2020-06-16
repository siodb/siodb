// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ClobStream.h"

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

/** String-based CLOB stream */
class StringClobStream : public ClobStream {
private:
    /**
     * Initializes object of class StringClobStream.
     * @param src Source stream.
     */
    explicit StringClobStream(const StringClobStream& src) noexcept;

public:
    /**
     * Initializes object of class StringClobStream.
     * @param s A string.
     */
    explicit StringClobStream(const std::string& s);

    /**
     * Initializes object of class StringClobStream.
     * @param s A string.
     */
    explicit StringClobStream(std::string&& s) noexcept;

    /**
     * Creates copy of this stream.
     * @return copy of this stream or nullptr of cloning of stream is not possible.
     */
    StringClobStream* clone() const override;

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t read(void* buffer, std::size_t bufferSize) override;

    /**
     * Rewinds stream to beginning.
     * @return Always returns true.
     */
    bool rewind() override;

private:
    /** Content string */
    const std::shared_ptr<const std::string> m_content;
};

}  // namespace siodb::iomgr::dbengine
