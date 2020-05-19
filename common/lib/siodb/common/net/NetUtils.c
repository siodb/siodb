// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NetUtils.h"

// System headers
#include <sys/socket.h>

int getSocketFamily(int sd)
{
    // Inspired by https://stackoverflow.com/questions/513298/how-can-i-determine-the-socket-family-from-the-socket-file-descriptor
    struct sockaddr sa;
    socklen_t len = 0;
    return getsockname(sd, &sa, &len) == 0 ? sa.sa_family : -1;
}
