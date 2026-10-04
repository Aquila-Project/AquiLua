#include "runtime.h"

#include "utils/macros.h"

int main(int args, char **argv) {
	aquiLua::Runtime runtime{};

	auto id = runtime.load(AQUILUA_ROOT_DIR "/sandbox/main.luau");

	runtime.call(id, "damage", 10);

	return EXIT_SUCCESS;
}
