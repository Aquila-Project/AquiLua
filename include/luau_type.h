/**************************************************************************/
/*  luau_type.h                                                           */
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
#include "converter.h"
#include "userdata.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace aquiLua {

// The Luau type of each C++ type push or check takes, for the definitions file. Same rule: no catch-all.

template <std::same_as<std::nullptr_t> T> const char *luau_type() {
	return "nil";
}

template <std::same_as<bool> T> const char *luau_type() {
	return "boolean";
}

template <typename T>
	requires((std::integral<T> || std::floating_point<T>) && !std::same_as<T, bool>)
const char *luau_type() {
	return "number";
}

template <typename T>
	requires(std::same_as<T, std::string> || std::same_as<T, std::string_view>)
const char *luau_type() {
	return "string";
}

// push turns a null const char * into nil
template <std::same_as<const char *> T> const char *luau_type() {
	return "string?";
}

template <std::same_as<lua_CFunction> T> const char *luau_type() {
	return "(...any) -> ...any";
}

template <Userdata T> const char *luau_type() {
	return UserType<T>::name;
}

template <Convertible T> const char *luau_type() {
	static_assert(requires { Converter<T>::luau_type; }, "Converter<T> needs a luau_type, see converter.h");
	return Converter<T>::luau_type;
}

// the Luau types a C++ function takes and returns
struct LuauSignature {
	std::vector<std::string> params;
	std::string result; // "()" when it returns nothing
};

template <typename R, typename... Args> LuauSignature make_signature() {
	LuauSignature signature{ .params = { luau_type<std::remove_cvref_t<Args>>()... }, .result = "()" };
	if constexpr (!std::is_void_v<R>) {
		signature.result = luau_type<std::remove_cvref_t<R>>();
	}
	return signature;
}

template <typename R, typename... Args> LuauSignature luau_signature(R (*)(Args...)) {
	return make_signature<R, Args...>();
}

// skips the context parameter, scripts never pass it
template <typename C, typename R, typename... Args> LuauSignature luau_context_signature(R (*)(C &, Args...)) {
	return make_signature<R, Args...>();
}

// without self, register_type adds it
template <typename C, typename R, typename... Args> LuauSignature luau_signature(R (C::*)(Args...)) {
	return make_signature<R, Args...>();
}

template <typename C, typename R, typename... Args> LuauSignature luau_signature(R (C::*)(Args...) const) {
	return make_signature<R, Args...>();
}

} // namespace aquiLua
