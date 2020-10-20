// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// CRT headers
#include <cstdint>

// STL headers
#include <ostream>
#include <string>

namespace siodb::utils {

/**
 * Dumps area of memory into string.
 * @param addr Memory address
 * @param length Memory area length.
 * @param stride Number of bytes per memory dump line, 1 to 256.
 * @return Memory dumped as string.
 */
std::string dumpMemoryToString(const void* addr, std::size_t length, std::size_t stride = 16);

/**
 * Dumps area of memory into stream.
 * @param addr Memory address
 * @param length Memory area length.
 * @param stride Number of bytes per memory dump line, 1 to 256.
 * @param os Output stream.
 */
void dumpMemoryToStream(const void* addr, std::size_t length, std::size_t stride, std::ostream& os);

/**
 * Breakpoint placeholder function.
 */
void debugPlaceholder() noexcept;

/**
 * Returns dummy debug counter.
 * @return Debug counter.
 */
std::uint64_t getDebugCounter() noexcept;

}  // namespace siodb::utils
