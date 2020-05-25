// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine::helpers {

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param fieldName Name of the field that is being deserialzied.
 * @param message Deserializarion error description.
 */
[[noreturn]] void reportDeserializationFailure(
        const char* className, const char* fieldName, const char* message);

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param fieldName Name of the field that is being deserialzied.
 * @param errorCode Deserializarion error code.
 */
[[noreturn]] void reportInvalidOrNotEnoughData(
        const char* className, const char* fieldName, int errorCode);

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param actualClassUuid Serialized UUID.
 * @param requiredClassUuid Required UUID.
 */
[[noreturn]] void reportClassUuidMismatch(const char* className,
        const std::uint8_t* actualClassUuid, const std::uint8_t* requiredClassUuid);

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param actualClassVersion Serialized data version.
 * @param supportedClassVersion Supported data version.
 */
[[noreturn]] void reportClassVersionMismatch(const char* className,
        std::uint32_t actualClassVersion, std::uint32_t supportedClassVersion);

}  // namespace siodb::iomgr::dbengine::helpers
