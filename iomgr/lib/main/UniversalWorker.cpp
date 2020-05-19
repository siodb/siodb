// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UniversalWorker.h"

namespace siodb::iomgr {

UniversalWorker::UniversalWorker(std::size_t workerId)
    : WorkerBase("UW", workerId)
{
    start();
}

UniversalWorker::~UniversalWorker()
{
}

void UniversalWorker::workerThreadMain()
{
    // Stub code
    while (!isExitRequested()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // TODO: implement UniversalWorker::workerThreadMain()
}

}  // namespace siodb::iomgr
