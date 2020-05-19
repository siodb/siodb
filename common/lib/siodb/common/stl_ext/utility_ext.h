// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <type_traits>
#include <utility>

namespace stdext {

/**
 * Converts scoped enumeration element into value of an underlying integer type.
 * @see https://stackoverflow.com/a/26809130/1540501
 * @param value Enumeration element.
 * @return Value of underlying type that corresponds to the give enumeration value.
 */
template<typename E>
inline constexpr std::enable_if_t<std::is_enum_v<E>, std::underlying_type_t<E>> underlying_value(
        const E value)
{
    return static_cast<std::underlying_type_t<E>>(value);
}

/**
 * Creates copy of the object using copy constructor.
 * @param src Source object.
 * @return Copy of the object.
 */
template<class T>
inline T copy(const T& src) noexcept(std::is_nothrow_copy_constructible_v<T>)
{
    return T(src);
}

/**
 * Casts reference to a const type to the reference to a mutable type.
 * @param obj A reference to a const type.
 * @return Reference to a mutable type.
 */
template<class T>
inline constexpr std::remove_const_t<T>& as_mutable(const T& obj) noexcept
{
    return const_cast<std::remove_const_t<T>&>(obj);
}

/**
 * Casts pointer to a const type to the pointer to a mutable type.
 * @param obj Pointer to a const type.
 * @return Pointer to a mutable type.
 */
template<class T>
inline constexpr std::remove_const_t<T>* as_mutable_ptr(const T* obj) noexcept
{
    return const_cast<std::remove_const_t<T>*>(obj);
}

}  // namespace stdext
