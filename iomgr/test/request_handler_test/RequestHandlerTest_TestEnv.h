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
    /**
     * Test User 0 - General usage (user with permissions for test db)
     * Test User 1 - General usage (user w/o permissions)
     * Test User 2 - UserPermissions.ShowPermissions_NormalUser (user with permissions for test db)
     */
    static constexpr std::size_t kTestUserCount = 3;

    static constexpr std::uint64_t kTestRequestId = 256;

public:
    TestEnvironment(const char* argv0);

    static auto getInstance() noexcept
    {
        return m_env->m_instance;
    }

    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandlerForNormalUser(
            std::size_t testUserIndex = 0);

    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandlerForUser(
            const std::string& userName);

    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandlerForSuperUser();

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

    static const auto& getTestUserName(std::size_t index)
    {
        return m_testUserNames.at(index);
    }

    static auto getTestUserId(std::size_t index)
    {
        return m_testUserIds.at(index);
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
    static std::unique_ptr<dbengine::RequestHandler> makeRequestHandler(std::uint32_t userId);

    const char* m_argv0;
    dbengine::InstancePtr m_instance;
    Pipes m_pipes;
    std::unique_ptr<siodb::io::InputStream> m_input;
    std::unique_ptr<siodb::io::OutputStream> m_output;
    std::string m_instanceFolder;
    static TestEnvironment* m_env;
    static std::array<std::string, kTestUserCount> m_testUserNames;
    static std::array<std::uint32_t, kTestUserCount> m_testUserIds;
    static std::string m_testDatabaseName;
    static std::string m_testDatabaseNameLowerCase;
};
