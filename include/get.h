/**************************************************************************/
/*  get.h                                                                 */
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
#include <string>
#include <type_traits>

namespace aquiLua {

template <std::same_as<bool> T> T get(lua_State *state, int index) {
	return lua_toboolean(state, index) != 0;
}

template <std::integral T>
	requires(!std::same_as<T, bool>)
T get(lua_State *state, int index) {
	if constexpr (std::is_signed_v<T> && sizeof(T) <= sizeof(int)) {
		return static_cast<T>(lua_tointeger(state, index));
	} else if constexpr (std::is_unsigned_v<T> && sizeof(T) <= sizeof(unsigned)) {
		return static_cast<T>(lua_tounsigned(state, index));
	} else {
		return static_cast<T>(lua_tonumber(state, index));
	}
}

template <std::floating_point T> T get(lua_State *state, int index) {
	return static_cast<T>(lua_tonumber(state, index));
}

template <std::same_as<std::string> T> T get(lua_State *state, int index) {
	size_t length = 0;
	const char *value = lua_tolstring(state, index, &length);
	return value != nullptr ? std::string(value, length) : std::string();
}

} // namespace aquiLua
