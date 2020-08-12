// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/Instance.h"
#include "dbengine/handlers/RequestHandler.h"

// Common project headers
#include <siodb/common/io/IODevice.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;

class TestEnvironment : public ::testing::Environment {
public:
    static constexpr std::uint64_t kTestRequestId = 256;

public:
    TestEnvironment(const char* argv0)
        : m_argv0(argv0)
    {
        m_env = this;
    }

    static dbengine::InstancePtr getInstance() noexcept
    {
        return m_env->m_instance;
    }

    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandler();

    typedef int Pipes[2];

    static const Pipes& getPipes() noexcept
    {
        return m_env->m_pipes;
    }

    static siodb::io::IODevice& getInputStream() noexcept
    {
        return *m_env->m_input;
    }

    static siodb::io::IODevice& getOutputStream() noexcept
    {
        return *m_env->m_output;
    }

    void SetUp() override;

    void TearDown() override;

private:
    dbengine::InstancePtr m_instance;
    Pipes m_pipes;
    const char* m_argv0;
    std::unique_ptr<siodb::io::IODevice> m_input;
    std::unique_ptr<siodb::io::IODevice> m_output;
    std::string m_instanceFolder;
    static TestEnvironment* m_env;
};
