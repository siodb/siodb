// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "WorkerBase.h"

namespace siodb::iomgr {

/** Worker class for performing data file IO operations */
class UniversalWorker : public WorkerBase {
public:
    /**
     * Initializes object of class UniversalWorker.
     * @param workerId Worker ID.
     */
    explicit UniversalWorker(std::size_t workerId);

    /** De-initializes object of class UniversalWorker. */
    virtual ~UniversalWorker();

    DECLARE_NONCOPYABLE(UniversalWorker);

private:
    /** Worker thread main function */
    void workerThreadMain() override;
};

}  // namespace siodb::iomgr
