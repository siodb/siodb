// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Boost headers
#include <boost/uuid/uuid.hpp>

// Make GCC happy on the CentOS/RHEL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <boost/uuid/uuid_generators.hpp>
#pragma GCC diagnostic pop

#include <boost/uuid/uuid_io.hpp>

namespace siodb {

using Uuid = boost::uuids::uuid;

}  // namespace siodb

namespace siodb::utils {

const Uuid& getZeroUuid() noexcept;

}  // namespace siodb::utils

namespace std {

// Specialization of the std::hash for Uuid
template<>
struct hash<siodb::Uuid> {
    /**
     * Hash calculation operator.
     * @param uuid UUID.
     * @return hash value.
     */
    std::size_t operator()(const siodb::Uuid& uuid) const noexcept
    {
        return boost::uuids::hash_value(uuid);
    }
};

}  // namespace std
