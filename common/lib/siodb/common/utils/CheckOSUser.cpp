// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CheckOSUser.h"

// Project headers
#include "../config/SiodbDefs.h"
#include "../stl_ext/system_error_ext.h"

// STL headers
#include <algorithm>
#include <sstream>
#include <vector>

// System headers
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace siodb::utils {

namespace {
constexpr long kDefaultInitialBufferSize = 256;

bool getPwd(uid_t uid, struct passwd& pwd, std::vector<char>& buffer)
{
    auto maxBufferSize = ::sysconf(_SC_GETPW_R_SIZE_MAX);
    if (maxBufferSize < 0) maxBufferSize = kDefaultInitialBufferSize;
    if (buffer.size() < std::size_t(maxBufferSize)) buffer.resize(maxBufferSize);

    struct passwd* ppwd = nullptr;
    while (true) {
        const int errorCode = ::getpwuid_r(uid, &pwd, buffer.data(), buffer.size(), &ppwd);
        if (errorCode == 0) break;
        if (errorCode == ERANGE) {
            buffer.resize(buffer.size() * 2);
            continue;
        }
        std::ostringstream err;
        err << "Can't get user information for the UID " << uid;
        stdext::throw_system_error(errorCode, err.str());
    }
    return (ppwd != nullptr);
}

}  // namespace

uid_t getOsUserId(const char* name)
{
    auto maxBufferSize = ::sysconf(_SC_GETPW_R_SIZE_MAX);
    if (maxBufferSize < 0) maxBufferSize = kDefaultInitialBufferSize;
    std::vector<char> buffer(maxBufferSize);
    struct passwd pwd;
    struct passwd* ppwd = nullptr;
    while (true) {
        const int errorCode = ::getpwnam_r(name, &pwd, buffer.data(), buffer.size(), &ppwd);
        if (errorCode == 0) break;
        if (errorCode == ERANGE) {
            buffer.resize(buffer.size() * 2);
            continue;
        }
        std::ostringstream err;
        err << "Can't get ID of the user " << name;
        stdext::throw_system_error(errorCode, err.str().c_str());
    }
    if (ppwd != nullptr) return pwd.pw_uid;
    std::ostringstream err;
    err << "User " << name << " doesn't exist.";
    throw std::runtime_error(err.str());
}

std::string getOsUserName(uid_t uid)
{
    struct passwd pwd;
    std::vector<char> buffer;
    if (getPwd(uid, pwd, buffer)) {
        return pwd.pw_name;
    }
    std::ostringstream err;
    err << "User doesn't exists: UID " << uid;
    throw std::runtime_error(err.str());
}

gid_t getOsGroupId(const char* name)
{
    auto maxBufferSize = ::sysconf(_SC_GETGR_R_SIZE_MAX);
    if (maxBufferSize < 0) maxBufferSize = kDefaultInitialBufferSize;
    std::vector<char> buffer(maxBufferSize);
    struct group grp;
    struct group* pgrp = nullptr;
    while (true) {
        const int errorCode = ::getgrnam_r(name, &grp, buffer.data(), buffer.size(), &pgrp);
        if (errorCode == 0) break;
        if (errorCode == ERANGE) {
            buffer.resize(buffer.size() * 2);
            continue;
        }
        std::ostringstream err;
        err << "Can't get ID of the group " << name;
        stdext::throw_system_error(errorCode, err.str().c_str());
    }
    if (pgrp != nullptr) return grp.gr_gid;
    std::ostringstream err;
    err << "Group " << name << " doesn't exist.";
    throw std::runtime_error(err.str());
}

std::string getOsGroupName(gid_t gid)
{
    auto maxBufferSize = ::sysconf(_SC_GETGR_R_SIZE_MAX);
    if (maxBufferSize < 0) maxBufferSize = kDefaultInitialBufferSize;
    std::vector<char> buffer(maxBufferSize);
    struct group grp;
    struct group* pgrp = nullptr;
    while (true) {
        const int errorCode = ::getgrgid_r(gid, &grp, buffer.data(), buffer.size(), &pgrp);
        if (errorCode == 0) break;
        if (errorCode == ERANGE) {
            buffer.resize(buffer.size() * 2);
            continue;
        }
        std::ostringstream err;
        err << "Can't get group information for the GID " << gid;
        stdext::throw_system_error(errorCode, err.str());
    }
    if (pgrp != nullptr) return grp.gr_name;
    std::ostringstream err;
    err << "Group doesn't exists: GID " << gid;
    throw std::runtime_error(err.str());
}

std::string checkUserBelongsToSiodbAdminGroup(uid_t uid, gid_t gid)
{
    const auto userName = getOsUserName(uid);

    // Obtain user groups
    // requires GLIBC >= 2.3.3 due to buffer overrun bug
    int ngroups = 2;
    std::vector<gid_t> groups(ngroups);
    if (::getgrouplist(userName.c_str(), gid, groups.data(), &ngroups) < 0) {
        groups.resize(ngroups);
        ::getgrouplist(userName.c_str(), gid, groups.data(), &ngroups);
    }

    // Obtain admin group ID
    const auto adminGroupId = getOsGroupId(kAdminGroupName);

    // Check if user belongs to the admin group
    if (std::find(groups.begin(), groups.end(), adminGroupId) == groups.end()) {
        std::ostringstream err;
        err << "User '" << userName << "' doesn't belong to the administrative group '"
            << kAdminGroupName << '\'';
        throw std::runtime_error(err.str());
    }

    return userName;
}

std::string getHomeDir()
{
    const char* homeEnv = getenv("HOME");
    if (homeEnv == nullptr) {
        const auto uid = ::getuid();

        struct passwd pwd;
        std::vector<char> buffer;
        if (getPwd(uid, pwd, buffer))
            return pwd.pw_dir;
        else
            stdext::throw_system_error("Can't get home directory");
    }

    return homeEnv;
}

}  // namespace siodb::utils
