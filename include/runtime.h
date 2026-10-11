/**************************************************************************/
/*  runtime.h                                                             */
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
#include "luacode.h"
#include "lualib.h"
#include "push.h"
#include "get.h"

#include "utils/macros.h"

#include <fstream>
#include <optional>
#include <type_traits>
#include <unordered_map>

#define INVALID_SCRIPT_ID 0

namespace aquiLua {

struct ScriptId {
	uint32_t id = INVALID_SCRIPT_ID;

	bool is_valid() const { return id != INVALID_SCRIPT_ID; }
};

template <typename R> using CallResult = std::conditional_t<std::is_void_v<R>, bool, std::optional<R>>;

class Runtime {
  public:
	Runtime();
	~Runtime();

	// delete copy
	Runtime(const Runtime &) = delete;
	Runtime &operator=(const Runtime &) = delete;

	// delete move
	Runtime(Runtime &&) = delete;
	Runtime &operator=(Runtime &&) = delete;

	bool create_vm();
	void destroy_vm();

	ScriptId load(const std::string &filename);

	template <typename R = void, typename... Args>
	CallResult<R> call(ScriptId id, const std::string &fct, Args... args) {
		if (!id.is_valid()) {
			fprintf(stderr, "Script is not valid.");
			return {};
		}

		if (m_state == nullptr) {
			fprintf(stderr, "State is invalid.");
			return {};
		}

		if (!m_script.contains(id.id)) {
			return {};
		}

		int ticket = m_script.find(id.id)->second;
		constexpr int result_count = std::is_void_v<R> ? 0 : 1;

		lua_getref(m_state, ticket); // stack : [M]
		lua_getfield(m_state, -1, fct.c_str()); // stack : [M, fn]
		if (!lua_isfunction(m_state, -1)) {
			fprintf(stderr, "%s is not a function.\n", fct.c_str());
			lua_pop(m_state, 2); // stack : []
			return {};
		}

		(push(m_state, args), ...); // stack : [M, fn, args...]
		int result = lua_pcall(m_state, sizeof...(Args), result_count, 0); // stack : [M, results...] or [M, error]
		if (result != LUA_OK) {
			fprintf(stderr, "%s\n", lua_tostring(m_state, -1));
			lua_pop(m_state, 2); // stack : []
			return {};
		}

		if constexpr (std::is_void_v<R>) {
			lua_pop(m_state, 1); // stack : []
			return true;
		} else {
			R value = get<R>(m_state, -1);
			lua_pop(m_state, 2); // stack : []
			return value;
		}
	}

  private:
	lua_State *m_state = nullptr;
	std::unordered_map<uint32_t, int> m_script;
	uint32_t m_next_id = 0;
};
}; // namespace aquiLua