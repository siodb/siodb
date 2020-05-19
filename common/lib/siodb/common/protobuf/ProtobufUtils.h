// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include <google/protobuf/message.h>

namespace siodb::protobuf {

/**
 * Converts the message to a string.
 * @param message Protobuf message.
 * @return The string representation of the message.
 */
std::string protobufMessageToString(const google::protobuf::Message& message);

}  // namespace siodb::protobuf
