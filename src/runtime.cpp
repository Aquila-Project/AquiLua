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

#include "runtime.h"

namespace aquiLua {
Runtime::Runtime() {
	create_vm();
};

Runtime::~Runtime() {
	destroy_vm();
}

bool Runtime::create_vm() {
	m_state = luaL_newstate();

	if (m_state != nullptr) {
		luaL_openlibs(m_state);
		return true;
	}

	return false;
}

void Runtime::destroy_vm() {
	if (m_state != nullptr) {
		lua_close(m_state);
		m_state = nullptr;
	}
}

ScriptId Runtime::load(const std::string &filename) {
	ScriptId script_id{};
	std::ifstream file(filename);
	if (!file) {
		fprintf(stderr, "There was an issue opening the file : %s\n", filename.c_str());
		return script_id;
	}

	std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	size_t bytecode_size = 0;
	char *bytecode = luau_compile(source.data(), source.size(), nullptr, &bytecode_size);

	std::string chunk_name = std::string("@") + filename; // runtime errors will point at the .luau file
	int status = luau_load(m_state, chunk_name.c_str(), bytecode, bytecode_size, 0); // stack : [chunk]

	free(bytecode);

	if (status != 0) {
		fprintf(stderr, "%s\n", lua_tostring(m_state, -1));
		lua_pop(m_state, 1); // stack : []
		return script_id;
	}

	status = lua_pcall(m_state, 0, 1, 0); // get whatever the fuck the file returns and keep it. stack : [M] or [error]
	if (status != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tostring(m_state, -1));
		lua_pop(m_state, 1); // stack : []
		return script_id;
	}

	if (!lua_istable(m_state, -1)) {
		fprintf(stderr, "%s must return a table of functions (return M).\n", filename.c_str());
		lua_pop(m_state, 1); // stack : []
		return script_id;
	}

	int ticket = lua_ref(m_state, -1); // get the id
	lua_pop(m_state, 1); // pop whats on the stack

	script_id.id = ++m_next_id;

	m_script[script_id.id] = ticket; // map the script id to lua ticket

	return script_id;
}

}; // namespace aquiLua