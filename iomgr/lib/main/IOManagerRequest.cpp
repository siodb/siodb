// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRequest.h"

// Project headers
#include "../dbengine/handlers/RequestHandler.h"

namespace siodb::iomgr {

std::atomic<std::uint64_t> IOManagerRequest::s_requestIdCounter(0);

void IOManagerRequest::execute() const
{
    m_requestHandler->executeRequest(*m_dbeRequest, m_requestId, m_responseId, m_statementCount);
}

}  // namespace siodb::iomgr
