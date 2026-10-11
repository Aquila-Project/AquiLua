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
#include "binding.h"
#include "push.h"
#include "get.h"
#include "userdata.h"

#include "utils/macros.h"

#include <fstream>
#include <optional>
#include <string>
#include <initializer_list>
#include <string_view>
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

	// name is a global, or "module.name" to put it in a native module (see require). doc is shown by luau-lsp
	template <auto Fn> void bind(const std::string &name, const char *doc = nullptr) {
		register_function(name, wrapper<Fn>, luau_signature(Fn), doc);
	}

	// Fn's first parameter (a C &) receives *context
	template <auto Fn>
	void bind(const std::string &name, typename ContextOf<decltype(Fn)>::type *context, const char *doc = nullptr) {
		register_function(name, wrapper_with_context<Fn>, luau_context_signature(Fn), doc, context);
	}

	// each method gets context as upvalue 1. a type can only be registered once
	template <Userdata T>
	bool register_type(std::initializer_list<Method> methods, void *context = nullptr, const char *doc = nullptr) {
		return register_type(UserType<T>::name, methods, context, doc);
	}

	// require("@name/path") runs directory/path.luau (or directory/path/init.luau) once and returns its result
	void add_alias(const std::string &name, const std::string &directory) { m_aliases[name] = directory; }

	// Types of everything bound so far, for luau-lsp: globals.d.luau (globals and userdata types, for
	// luau-lsp.types.definitionFiles) and <module>.luau per native module (for a .luaurc alias named after it)
	bool write_types(const std::string &directory) const;

	ScriptId load(const std::string &filename);
	// name only shows up in error messages, e.g. a virtual path
	ScriptId load_source(const std::string &name, std::string_view source);
	void unload(ScriptId id);

	bool has_function(ScriptId id, const std::string &name);

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
	// functions bound as "module.name". only modules loaded by require can require them, so a Luau API written on
	// top of them is all scripts see
	struct NativeModule {
		int table = LUA_NOREF;
		std::string fields; // its Luau type, one line per function
	};

	void register_function(const std::string &name, lua_CFunction function, const LuauSignature &signature,
						   const char *doc, void *context = nullptr);
	bool register_type(const std::string &name, std::initializer_list<Method> methods, void *context, const char *doc);

	void push_function(lua_CFunction function, const char *name, void *context);

	static int require(lua_State *state);

	lua_State *m_state = nullptr;
	std::unordered_map<uint32_t, int> m_script;
	uint32_t m_next_id = 0;
	std::string m_definitions;
	std::unordered_map<std::string, NativeModule> m_modules;
	std::unordered_map<std::string, std::string> m_aliases;
	std::unordered_map<std::string, int> m_required; // file -> what it returned, LUA_NOREF while it runs
};
}; // namespace aquiLua