/**************************************************************************/
/*  check.h                                                               */
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
#include "lualib.h"
#include "converter.h"
#include "userdata.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>

namespace aquiLua {

template <std::same_as<bool> T> T check(lua_State *state, int index) {
	return luaL_checkboolean(state, index) != 0;
}

template <std::integral T>
	requires(!std::same_as<T, bool>)
T check(lua_State *state, int index) {
	if constexpr (std::is_signed_v<T> && sizeof(T) <= sizeof(int)) {
		return static_cast<T>(luaL_checkinteger(state, index));
	} else if constexpr (std::is_unsigned_v<T> && sizeof(T) <= sizeof(unsigned)) {
		return static_cast<T>(luaL_checkunsigned(state, index));
	} else {
		return static_cast<T>(luaL_checknumber(state, index));
	}
}

template <std::floating_point T> T check(lua_State *state, int index) {
	return static_cast<T>(luaL_checknumber(state, index));
}

template <std::same_as<std::string> T> T check(lua_State *state, int index) {
	size_t length = 0;
	const char *value = luaL_checklstring(state, index, &length);
	return std::string(value, length);
}

// Userdata types, see UserType in userdata.h. Returns a copy, so a T & parameter won't compile: take T or const T &.
template <Userdata T> T check(lua_State *state, int index) {
	return *check_userdata<T>(state, index, UserType<T>::name);
}

template <Convertible T> T check(lua_State *state, int index) {
	return Converter<T>::check(state, index);
}

} // namespace aquiLua
