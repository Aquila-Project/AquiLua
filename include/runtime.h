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

#include "utils/macros.h"

#include <fstream>
#include <unordered_map>

#define INVALID_SCRIPT_ID 0

namespace aquiLua {

struct ScriptId {
	uint32_t id = INVALID_SCRIPT_ID;

	bool is_valid() const { return id != INVALID_SCRIPT_ID; }
};

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

	template <typename... Args> bool call(ScriptId id, const std::string &fct, Args... args) {
		if (!id.is_valid()) {
			fprintf(stderr, "Script is not valid.");
			return false;
		}

		if (m_state == nullptr) {
			fprintf(stderr, "State is invalid.");
			return false;
		}

		if (!m_script.contains(id.id)) {
			return false;
		}

		int ticket = m_script.find(id.id)->second;

		lua_getref(m_state, ticket);
		lua_getfield(m_state, -1, fct.c_str());
		lua_isfunction(m_state, ticket);

		(push(m_state, args), ...);
		int result = lua_pcall(m_state, sizeof...(Args), 0, 0);
		if (result != LUA_OK) {
			fprintf(stderr, "Problem");
			return false;
		}

		lua_pop(m_state, 1);

		return true;
	}

  private:
	lua_State *m_state = nullptr;
	std::unordered_map<uint32_t, int> m_script;
	uint32_t m_next_id = 0;
};
}; // namespace aquiLua