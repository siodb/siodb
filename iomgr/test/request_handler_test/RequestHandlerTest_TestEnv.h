// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/Instance.h"
#include "dbengine/handlers/RequestHandler.h"

// Common project headers
#include <siodb/common/io/InputOutputStream.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;

class TestEnvironment : public ::testing::Environment {
public:
    static constexpr std::uint64_t kTestRequestId = 256;

public:
    TestEnvironment(const char* argv0);

    static auto getInstance() noexcept
    {
        return m_env->m_instance;
    }

    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandler(bool asSuperUser = true);

    typedef int Pipes[2];

    static const auto& getPipes() noexcept
    {
        return m_env->m_pipes;
    }

    static auto& getInputStream() noexcept
    {
        return *m_env->m_input;
    }

    static auto& getOutputStream() noexcept
    {
        return *m_env->m_output;
    }

    static const auto& getTestUserName() noexcept
    {
        return m_testUserName;
    }

    static auto getTestUserId() noexcept
    {
        return m_testUserId;
    }

    static const auto& getTestDatabaseName() noexcept
    {
        return m_testDatabaseName;
    }

    static const auto& getTestDatabaseNameLowerCase() noexcept
    {
        return m_testDatabaseNameLowerCase;
    }

    void SetUp() override;

    void TearDown() override;

private:
    const char* m_argv0;
    dbengine::InstancePtr m_instance;
    Pipes m_pipes;
    std::unique_ptr<siodb::io::InputStream> m_input;
    std::unique_ptr<siodb::io::OutputStream> m_output;
    std::string m_instanceFolder;
    static TestEnvironment* m_env;
    static std::string m_testUserName;
    static std::uint32_t m_testUserId;
    static std::string m_testDatabaseName;
    static std::string m_testDatabaseNameLowerCase;
};
