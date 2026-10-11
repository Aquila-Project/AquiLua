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
#include <utility>

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

// "(first, Args...) -> R". first is for a method's self
template <typename R, typename... Args> std::string luau_signature(std::string first = {}) {
	std::string params = std::move(first);
	auto add = [&params](const char *type) {
		if (!params.empty()) {
			params += ", ";
		}
		params += type;
	};
	(add(luau_type<std::remove_cvref_t<Args>>()), ...);

	if constexpr (std::is_void_v<R>) {
		return "(" + params + ") -> ()";
	} else {
		return "(" + params + ") -> " + luau_type<std::remove_cvref_t<R>>();
	}
}

template <typename R, typename... Args> std::string luau_function_type(R (*)(Args...)) {
	return luau_signature<R, Args...>();
}

// skips the context parameter, scripts never pass it
template <typename C, typename R, typename... Args> std::string luau_context_function_type(R (*)(C &, Args...)) {
	return luau_signature<R, Args...>();
}

template <typename C, typename R, typename... Args> std::string luau_method_type(R (C::*)(Args...), const char *self) {
	return luau_signature<R, Args...>(self);
}

template <typename C, typename R, typename... Args>
std::string luau_method_type(R (C::*)(Args...) const, const char *self) {
	return luau_signature<R, Args...>(self);
}

} // namespace aquiLua
