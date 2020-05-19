// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Unaligned data access
#if !defined(SIODB_FORCE_PROCESSOR_NEUTRAL_CODE) \
        && (defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_AMD64))
#define SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
#endif
