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

// the context a function was registered with (upvalue 1)
template <typename T> T &context(lua_State *state) {
	return *static_cast<T *>(lua_touserdata(state, lua_upvalueindex(1)));
}

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

// fn(C &context, args...)
template <typename C, typename R, typename... Args, std::size_t... I>
int invoke_with_context_impl(lua_State *state, R (*fn)(C &, Args...), std::index_sequence<I...>) {
	return push_result(state, [&] { return fn(context<C>(state), check<std::remove_cvref_t<Args>>(state, I + 1)...); });
}

template <typename C, typename R, typename... Args> int invoke_with_context(lua_State *state, R (*fn)(C &, Args...)) {
	return invoke_with_context_impl(state, fn, std::index_sequence_for<Args...>{});
}

template <auto Fn> int wrapper_with_context(lua_State *state) {
	return invoke_with_context(state, Fn);
}

template <typename> struct ContextOf;

template <typename C, typename R, typename... Args> struct ContextOf<R (*)(C &, Args...)> {
	using type = C;
};

template <auto Resolve, auto Fn, typename R, typename... Args, std::size_t... I>
int invoke_method_impl(lua_State *state, std::type_identity<R(Args...)>, std::index_sequence<I...>) {
	auto *self = Resolve(state);
	return push_result(state, [&] { return (self->*Fn)(check<std::remove_cvref_t<Args>>(state, I + 2)...); });
}

template <auto Resolve, auto Fn, typename C, typename R, typename... Args>
int invoke_method(lua_State *state, R (C::*)(Args...)) {
	return invoke_method_impl<Resolve, Fn>(state, std::type_identity<R(Args...)>{}, std::index_sequence_for<Args...>{});
}

template <auto Resolve, auto Fn, typename C, typename R, typename... Args>
int invoke_method(lua_State *state, R (C::*)(Args...) const) {
	return invoke_method_impl<Resolve, Fn>(state, std::type_identity<R(Args...)>{}, std::index_sequence_for<Args...>{});
}

template <auto Resolve, auto Fn> int method(lua_State *state) {
	return invoke_method<Resolve, Fn>(state, Fn);
}

} // namespace aquiLua
