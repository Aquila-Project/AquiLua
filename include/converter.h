/**************************************************************************/
/*  converter.h                                                           */
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

namespace aquiLua {

// specialize for value types with their own Luau representation, e.g. a vec3 as a Luau vector:
//   template <> struct aquiLua::Converter<Vec3> { static void push(...); static Vec3 check(...); static Vec3 get(...); };
template <typename T> struct Converter;

template <typename T>
concept Convertible = requires(lua_State *state, const T &value) {
	Converter<T>::push(state, value);
	{ Converter<T>::check(state, 1) } -> std::convertible_to<T>;
	{ Converter<T>::get(state, 1) } -> std::convertible_to<T>;
};

} // namespace aquiLua
