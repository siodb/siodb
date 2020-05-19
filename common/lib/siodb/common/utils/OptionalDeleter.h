// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::utils {

/**
 * Optional deleter class. Can be used to construct smart pointer
 * that optionally owns its resource.
 * @tparam T Controlled data type.
 */
template<class T>
class OptionalDeleter {
public:
    /**
     * Initializes object of class OptionalDeleter.
     * @param isOwner Resource ownership flag.
     */
    OptionalDeleter(bool isOwner) noexcept
        : m_isOwner(isOwner)
    {
    }

    /**
     * Returns ownership flag.
     * @return ownership flag.
     */
    bool isOwner() const noexcept
    {
        return m_isOwner;
    }

    /** Gains ownership */
    void gainOwnership() noexcept
    {
        m_isOwner = true;
    }

    /** Releases ownership */
    void releaseOwnership() noexcept
    {
        m_isOwner = false;
    }

    /**
     * Deletes given object if it is owned.
     * @param obj An object.
     */
    void operator()(T* obj) const noexcept
    {
        if (m_isOwner) delete obj;
    }

private:
    /** Ownership flag */
    bool m_isOwner;
};

}  // namespace siodb::utils
