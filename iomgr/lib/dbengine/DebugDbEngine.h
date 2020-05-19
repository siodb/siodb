// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

/** Define this to get extra debug output from the ULI */
//#define SIODB_DEBUG_ULI

/** Define this to get extra debug output from the block registry */
//#define SIODB_DEBUG_BREG

////////////////////////// Logging macros /////////////////////////////////////////////////////////

#ifdef SIODB_DEBUG_ULI
#define ULI_DBG_LOG_TRACE(x) DBG_LOG_TRACE(x)
#define ULI_DBG_LOG_DEBUG(x) DBG_LOG_DEBUG(x)
#define ULI_DBG_LOG_INFO(x) DBG_LOG_INFO(x)
#define ULI_DBG_LOG_WARNING(x) DBG_LOG_WARNING(x)
#define ULI_DBG_LOG_ERROR(x) DBG_LOG_ERROR(x)
#define ULI_DBG_LOG_FATAL(x) DBG_LOG_FATAL(x)
#else
#define ULI_DBG_LOG_TRACE(x) DBG_LOG_NO_OUTPUT(x)
#define ULI_DBG_LOG_DEBUG(x) DBG_LOG_NO_OUTPUT(x)
#define ULI_DBG_LOG_INFO(x) DBG_LOG_NO_OUTPUT(x)
#define ULI_DBG_LOG_WARNING(x) DBG_LOG_NO_OUTPUT(x)
#define ULI_DBG_LOG_ERROR(x) DBG_LOG_NO_OUTPUT(x)
#define ULI_DBG_LOG_FATAL(x) DBG_LOG_NO_OUTPUT(x)
#endif  // SIODB_DEBUG_ULI

#ifdef SIODB_DEBUG_BREG
#define BREG_DBG_LOG_TRACE(x) DBG_LOG_TRACE(x)
#define BREG_DBG_LOG_DEBUG(x) DBG_LOG_DEBUG(x)
#define BREG_DBG_LOG_INFO(x) DBG_LOG_INFO(x)
#define BREG_DBG_LOG_WARNING(x) DBG_LOG_WARNING(x)
#define BREG_DBG_LOG_ERROR(x) DBG_LOG_ERROR(x)
#define BREG_DBG_LOG_FATAL(x) DBG_LOG_FATAL(x)
#else
#define BREG_DBG_LOG_TRACE(x) DBG_LOG_NO_OUTPUT(x)
#define BREG_DBG_LOG_DEBUG(x) DBG_LOG_NO_OUTPUT(x)
#define BREG_DBG_LOG_INFO(x) DBG_LOG_NO_OUTPUT(x)
#define BREG_DBG_LOG_WARNING(x) DBG_LOG_NO_OUTPUT(x)
#define BREG_DBG_LOG_ERROR(x) DBG_LOG_NO_OUTPUT(x)
#define BREG_DBG_LOG_FATAL(x) DBG_LOG_NO_OUTPUT(x)
#endif  // SIODB_DEBUG_BREG
