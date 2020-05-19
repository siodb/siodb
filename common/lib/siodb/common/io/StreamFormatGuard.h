// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../utils/HelperMacros.h"

// STL headers
#include <ios>

namespace siodb::io {

/** Stores formatting information of stream and restores after object destruction */
class StreamFormatGuard final {
public:
    /**
     * Initialized object of class StreamFormatGuard.
     * Saves current format setting of the stream.
     * @param ios IOS
     */
    explicit StreamFormatGuard(std::ios& ios)
        : m_ios(ios)
        , m_saveFormatIos(nullptr)
    {
        m_saveFormatIos.copyfmt(m_ios);
    }

    /** 
     * De-initializes object of class StreamFormatGuard.
     * Restores format settings of the stream.
     */
    ~StreamFormatGuard()
    {
        m_ios.copyfmt(m_saveFormatIos);
    }

    DECLARE_NONCOPYABLE(StreamFormatGuard);

private:
    /** Stream object. */
    std::ios& m_ios;
    /** Supplementary stream object which keeps saved formatting information. */
    std::ios m_saveFormatIos;
};

}  // namespace siodb::io
