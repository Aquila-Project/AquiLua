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

void Runtime::push_function(lua_CFunction function, const char *name, void *context) {
	if (context != nullptr) {
		lua_pushlightuserdata(m_state, context); // just a raw pointer, lua doesnt own it. stack : [context]
		lua_pushcclosure(m_state, function, name, 1); // glue the context to the function. stack : [fn]
	} else {
		lua_pushcfunction(m_state, function, name); // stack : [fn]
	}
}

void Runtime::register_function(const std::string &name, lua_CFunction function, void *context) {
	push_function(function, name.c_str(), context); // stack : [fn]
	lua_setglobal(m_state, name.c_str()); // stack : []
}

bool Runtime::register_type(const std::string &name, std::initializer_list<luaL_Reg> methods, void *context) {
	if (!luaL_newmetatable(m_state, name.c_str())) { // name taken, it pushed the old one. stack : [metatable]
		fprintf(stderr, "Type %s is already registered.\n", name.c_str());
		lua_pop(m_state, 1); // stack : []
		return false;
	}

	lua_newtable(m_state); // stack : [metatable, methods]
	for (const luaL_Reg &method : methods) {
		push_function(method.func, method.name, context); // stack : [metatable, methods, fn]
		lua_setfield(m_state, -2, method.name); // stack : [metatable, methods]
	}
	lua_setfield(m_state, -2, "__index"); // stack : [metatable] and mt.__index = methods

	lua_pushstring(m_state, name.c_str()); // stack : [metatable, name]
	lua_setfield(m_state, -2, "__type"); // stack : [metatable] and mt.__type = name, so typeof() knows it

	lua_pop(m_state, 1); // its in the registry now, under name. stack : []
	return true;
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