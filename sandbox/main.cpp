/**************************************************************************/
/*  main.cpp                                                              */
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

#include "utils/macros.h"

#include <stdexcept>
#include <string>
#include <unordered_map>

// ! TODO: test only - delete later
#pragma region BINDING TEST
void say(const std::string &message, int a, int b) {
	int c = a + b;
	std::printf("%s and %d\n", message.c_str(), c);
}

double test(double a, double b) {
	return a + b;
}

struct Counter {
	int value = 0;

	void increment() { value++; }
	int get() const { return value; }
};

using CounterMap = std::unordered_map<uint32_t, Counter>;

struct CounterHandle {
	uint32_t id;
};

AQUILUA_USERTYPE(CounterHandle, "Counter");

// turns self (argument 1) into the real Counter. aquiLua::method uses it for every Counter method
Counter *check_counter(lua_State *state) {
	auto handle = aquiLua::check<CounterHandle>(state, 1);
	auto &counters = aquiLua::context<CounterMap>(state);
	auto it = counters.find(handle.id);
	if (it == counters.end()) {
		luaL_error(state, "counter %u was destroyed", handle.id);
	}
	return &it->second;
}

CounterHandle get_counter(CounterMap &counters, uint32_t id) {
	if (!counters.contains(id)) {
		throw std::runtime_error("no counter with id " + std::to_string(id)); // luau turns it into a script error
	}
	return CounterHandle{ id };
}

int main(int args, char **argv) {
	CounterMap counters;
	aquiLua::Runtime runtime{};

	AQUILUA_BIND(runtime, say);
	AQUILUA_BIND(runtime, test);

	counters[1] = Counter{}; // the engine creates the counter before any script sees it
	AQUILUA_BIND(runtime, get_counter, &counters);
	runtime.register_type<CounterHandle>(
		{ AQUILUA_METHOD(check_counter, Counter, increment), AQUILUA_METHOD(check_counter, Counter, get) }, &counters);

	auto id = runtime.load(AQUILUA_ROOT_DIR "/sandbox/main.luau");

	auto health = runtime.call<double>(id, "damage", 10);
	if (health) {
		printf("C++: health is %g\n", *health);
	}

	runtime.call(id, "bump");
	printf("C++: counter 1 is %d\n", counters[1].value); // the engine sees the script's change

	counters.erase(1); // the engine destroys it while the script still holds a handle
	bool bumped = runtime.call(id, "bump");
	printf("C++: bump after destroy %s\n", bumped ? "worked" : "failed");

	return EXIT_SUCCESS;
}
