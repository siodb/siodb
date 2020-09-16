// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// CRT Headers
#include <ctime>

// STL headers
#include <iostream>

// System headers
#include <unistd.h>

int main()
{
    std::time_t t = 0;
    while (::time(&t) > 0) {
        std::cout << t << ": I am alive" << std::endl;
        ::usleep(30 * 1000000);
    }
    return 0;
}
