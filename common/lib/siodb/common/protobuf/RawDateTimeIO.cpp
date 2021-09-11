// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RawDateTimeIO.h"

namespace siodb::protobuf {

bool readRawDateTime(google::protobuf::io::CodedInputStream& is, RawDateTime& rawDateTime)
{
    std::uint8_t buffer[RawDateTime::kSerializedSize];
    if (!is.ReadRaw(buffer, RawDateTime::kDatePartSerializedSize)) return false;
    rawDateTime.deserializeDatePart(buffer);
    if (rawDateTime.m_datePart.m_hasTimePart) {
        if (!is.ReadRaw(buffer + RawDateTime::kDatePartSerializedSize,
                    RawDateTime::kTimePartSerializedSize))
            return false;
        rawDateTime.deserialize(buffer, sizeof(buffer));
    }
    return true;
}

void writeRawDateTime(google::protobuf::io::CodedOutputStream& os, const RawDateTime& rawDateTime)
{
    std::uint8_t buffer[RawDateTime::kSerializedSize];
    const auto p = rawDateTime.serialize(buffer);
    os.WriteRaw(buffer, static_cast<int>(p - buffer));
}

}  // namespace siodb::protobuf
