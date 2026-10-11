/**************************************************************************/
/*  push.h                                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                                AQUILUA                                 */
/*                 Luau scripting for the Aquila Project                  */
/*               https://github.com/Aquila-Project/AquiLua                */
/**************************************************************************/
/* Copyright (c) 2026-present Alexandru DINTE.                            */
/* Licensed under the MIT License, see LICENSE.md.                        */
/**************************************************************************/

#pragma once

#include "lua.h"

#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace aquiLua {

// One push per supported C++ type. A type with no overload fails at compile time.

inline void push(lua_State *state, std::nullptr_t) {
	lua_pushnil(state);
}

// Template so pointers and other types don't silently convert to bool.
template <std::same_as<bool> T> void push(lua_State *state, T value) {
	lua_pushboolean(state, value);
}

template <std::integral T>
	requires(!std::same_as<T, bool>)
void push(lua_State *state, T value) {
	if constexpr (std::is_signed_v<T> && sizeof(T) <= sizeof(int)) {
		lua_pushinteger(state, static_cast<int>(value));
	} else if constexpr (std::is_unsigned_v<T> && sizeof(T) <= sizeof(unsigned)) {
		lua_pushunsigned(state, static_cast<unsigned>(value));
	} else {
		// 64-bit values become a regular Luau number (a double), exact up to 2^53
		lua_pushnumber(state, static_cast<double>(value));
	}
}

template <std::floating_point T> void push(lua_State *state, T value) {
	lua_pushnumber(state, static_cast<double>(value));
}

// A null const char * pushes nil.
inline void push(lua_State *state, const char *value) {
	lua_pushstring(state, value);
}

// Also takes std::string.
inline void push(lua_State *state, std::string_view value) {
	lua_pushlstring(state, value.data(), value.size());
}

inline void push(lua_State *state, lua_CFunction function) {
	lua_pushcfunction(state, function, nullptr);
}

} // namespace aquiLua
