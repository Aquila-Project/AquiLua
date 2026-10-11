/**************************************************************************/
/*  macros.h                                                              */
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

namespace aquiLua {

#define LUA_CALL_CHECK_RESULT(L, call)                                                          \
	do {                                                                                        \
		int lua_call_result_ = (call);                                                          \
		if (lua_call_result_ != 0) {                                                            \
			const char *lua_err_ = lua_tostring((L), -1);                                       \
			fprintf(stderr, "[Lua] Call failed: %s | Error: %s | File: %s | Line: %d\n", #call, \
					lua_err_ ? lua_err_ : "(no message)", __FILE__, __LINE__);                  \
			lua_pop((L), 1);                                                                    \
		}                                                                                       \
	} while (0)

// AQUILUA_BIND(runtime, say) or AQUILUA_BIND(runtime, get_counter, &counters).
// the Luau name is the C++ name
#define AQUILUA_BIND(runtime, fn, ...) (runtime).bind<fn>(#fn __VA_OPT__(, ) __VA_ARGS__)

// AQUILUA_METHOD(check_counter, Counter, increment),
// for the list given to register_type
#define AQUILUA_METHOD(resolve, type, fn) aquiLua::make_method<resolve, &type::fn>(#fn)

// AQUILUA_USERTYPE(CounterHandle, "Counter"), at global scope
#define AQUILUA_USERTYPE(type, lua_name)              \
	template <> struct aquiLua::UserType<type> {      \
		static constexpr const char *name = lua_name; \
	}

} // namespace aquiLua
