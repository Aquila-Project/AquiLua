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

int main(int args, char **argv) {
	aquiLua::Runtime runtime{};

	auto id = runtime.load(AQUILUA_ROOT_DIR "/sandbox/main.luau");

	runtime.call(id, "damage", 10);

	return EXIT_SUCCESS;
}
