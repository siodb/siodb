// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/**
 * Checks registry record UUIDs for uniqueness.
 * @throw std::runtime_error if non-unique UUID encountered.
 */
void checkRegistryRecordUuids();

}  // namespace siodb::iomgr::dbengine
