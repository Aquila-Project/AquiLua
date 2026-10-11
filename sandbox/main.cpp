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

// ! TODO: test only - delete later
#pragma region BINDING TEST
void say(const std::string &message, int a, int b) {
	int c = a + b;
	std::printf("%s and %d\n", message.c_str(), c);
}

double test(double a, double b) {
	return a + b;
}

int main(int args, char **argv) {
	aquiLua::Runtime runtime{};

	runtime.bind<say>("say");
	runtime.bind<test>("test");

	auto id = runtime.load(AQUILUA_ROOT_DIR "/sandbox/main.luau");

	auto health = runtime.call<double>(id, "damage", 10);
	if (health) {
		printf("C++: health is %g\n", *health);
	}

	return EXIT_SUCCESS;
}
