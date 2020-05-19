// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::utils {

/** Message severity class */
enum class MessageSeverity {
    /** Debug output, like printing out variable values or step-by-step execution progress */
    kDebug,

    /** Detailed progress of the single operation */
    kTrace,

    /** General informational and progress messages */
    kInfo,

    /** Untypical situation or error, where program can continue to run normally */
    kWarning,

    /** Error that requires external attention */
    kError,

    /** Error that causes program to exit */
    kFatal,
};

}  // namespace siodb::utils
