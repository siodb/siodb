// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ProtobufUtils.h"

// Protobuf headers
#include <google/protobuf/util/json_util.h>

namespace siodb::protobuf {

std::string protobufMessageToString(const google::protobuf::Message& message)
{
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    std::string s;
    google::protobuf::util::MessageToJsonString(message, &s, options);
    return s;
}

}  // namespace siodb::protobuf
