/**************************************************************************/
/*  userdata.h                                                            */
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

#include <concepts>
#include <new>
#include <type_traits>

namespace aquiLua {

template <typename T> struct UserType;

template <typename T>
concept Userdata = requires {
	{ UserType<T>::name } -> std::convertible_to<const char *>;
};

// Creates a userdata holding value, with the metatable Runtime::register_type made for type. stack : [ud]
template <typename T> T *new_userdata(lua_State *state, const char *type, T value) {
	static_assert(std::is_trivially_destructible_v<T>, "userdata destructors are not supported yet");

	void *memory = lua_newuserdata(state, sizeof(T)); // asks lua for raw memory. stack : [ud]
	T *object = new (memory) T(value); // placement new in lua allocated memory. THIS MEMORY BELONGS TO LUA !!

	luaL_getmetatable(state, type); // the one register_type made. stack : [ud, metatable]
	if (lua_isnil(state, -1)) {
		luaL_error(state, "type %s is not registered", type);
	}
	lua_setmetatable(state, -2); // stack : [ud]

	return object;
}

template <typename T> T *check_userdata(lua_State *state, int index, const char *type) {
	return static_cast<T *>(luaL_checkudata(state, index, type));
}

} // namespace aquiLua
