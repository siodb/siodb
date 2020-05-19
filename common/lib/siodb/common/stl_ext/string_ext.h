// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "cstdint_ext.h"

// STL headers
#include <string>

namespace stdext {

/**
 * Converts string to unsigned integer.
 * @param str A string to be converted.
 * @param[out] pos Address of variable to save index of the fist non-processed character.
 * @param base Numeric base.
 * @return Converted number.
 * @throw std::invalid_argument
 * @throw std::out_of_range
 * @see https://en.cppreference.com/w/cpp/string/basic_string/stoul
 * @see https://stackoverflow.com/a/8715855/1540501
 */
unsigned int stou(const std::string& str, std::size_t* pos = nullptr, int base = 10);

/**
 * Converts signed integer value to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(int value, int base);

/**
 * Converts signed long integer va.lue to string with same content in the given numeric base.
 * @param value A numeric value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(long value, int base);

/**
 * Converts signed long long integer va.lue to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(long long value, int base);

/**
 * Converts unsigned integer value to string with same content using in the given base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(unsigned value, int base);

/**
 * Converts unsigned long integer va.lue to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(unsigned long value, int base);

/**
 * Converts unsigned long long integer va.lue to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(unsigned long long value, int base);

/**
 * Converts 128-bit signed integer value to string with same content.
 * @param value A value to convert.
 * @return A string holding the converted value.
 */
std::string to_string(const int128_t& value);

/**
 * Converts 128-bit unsigned integer value to string with same content.
 * @param value A value to convert.
 * @return A string holding the converted value.
 */
std::string to_string(const uint128_t& value);

/**
 * Converts 128-bit signed integer va.lue to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(const int128_t& value, int base);

/**
 * Converts 128-bit unsigned integer va.lue to string with same content in the given numeric base.
 * @param value A value to convert.
 * @param base Numeric base. Supported values: 8, 10, 16.
 * @return A string holding the converted value.
 * @throw std::invalid_argument if numeric base is not supported.
 */
std::string to_string(const uint128_t& value, int base);

}  // namespace stdext
