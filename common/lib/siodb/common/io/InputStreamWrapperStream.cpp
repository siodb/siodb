// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "InputStreamWrapperStream.h"

// CRT headers
#include <cerrno>

namespace siodb::io {

InputStreamWrapperStream::~InputStreamWrapperStream()
{
    if (isValid()) close();
}

bool InputStreamWrapperStream::isValid() const noexcept
{
    return m_in && m_in->isValid();
}

int InputStreamWrapperStream::close() noexcept
{
    if (m_in) {
        m_in = nullptr;
        return 0;
    }
    errno = EIO;
    return -1;
}

}  // namespace siodb::io
