// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine::helpers {

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param fieldName Name of the field that is being deserialzied.
 * @param errorCode Deserializarion error code.
 */
[[noreturn]] void reportDeserializationFailure(
        const char* className, const char* fieldName, int errorCode);

/**
 * Throws exception with deserialization failure details.
 * @param className Name of the object class that is being deserialzied.
 * @param fieldName Name of the field that is being deserialzied.
 * @param message Deserializarion error description.
 */
[[noreturn]] void reportDeserializationFailure(
        const char* className, const char* fieldName, const char* message);

}  // namespace siodb::iomgr::dbengine::helpers
