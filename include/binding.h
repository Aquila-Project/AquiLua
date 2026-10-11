/**************************************************************************/
/*  runtime.cpp                                                           */
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

#include "push.h"
#include "check.h"
#include "lua.h"

#include <utility>
#include <type_traits>

namespace aquiLua {

// pushes what call returns, if anything. returns how many values were pushed
template <typename F> int push_result(lua_State *state, F &&call) {
	if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
		call();
		return 0;
	} else {
		push(state, call());
		return 1;
	}
}

template <typename R, typename... Args, std::size_t... I>
int invoke_impl(lua_State *state, R (*fn)(Args...), std::index_sequence<I...>) {
	return push_result(state, [&] { return fn(check<std::remove_cvref_t<Args>>(state, I + 1)...); });
}

template <typename R, typename... Args> int invoke(lua_State *state, R (*fn)(Args...)) {
	return invoke_impl(state, fn, std::index_sequence_for<Args...>{});
}

template <auto Fn> int wrapper(lua_State *state) {
	return invoke(state, Fn);
}

} // namespace aquiLua
