// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Determines socket family (domain).
 * @param fd Socket file descriptor.
 * @return Socket family (domain) or -1 on error.
 */
int getSocketFamily(int fd);

#ifdef __cplusplus
}  // extern "C"
#endif
