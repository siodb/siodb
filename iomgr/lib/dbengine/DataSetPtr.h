// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class DataSet;

/** Data set shared pointer shortcut type */
using DataSetPtr = std::shared_ptr<DataSet>;

/** Constant data set shared pointer shortcut type */
using ConstDataSetPtr = std::shared_ptr<const DataSet>;

}  // namespace siodb::iomgr::dbengine
