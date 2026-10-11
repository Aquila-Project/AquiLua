--!strict

----------------------------------------------------------------------------
--  main.luau                                                             --
----------------------------------------------------------------------------
--                         This file is part of:                          --
--                                AQUILUA                                 --
--                 Luau scripting for the Aquila Project                  --
--               https://github.com/Aquila-Project/AquiLua                --
----------------------------------------------------------------------------
-- Copyright (c) 2026-present Alexandru DINTE.                            --
-- Licensed under the MIT License, see LICENSE.md.                        --
----------------------------------------------------------------------------

type Player = {
	name: string,
	health: number,
}

local function describe(p: Player): string
	return p.name .. " has " .. tostring(p.health) .. " health"
end

local player: Player = { name = "hero", health = 100 }
print("in luau: " .. describe(player))
say("C++ function called from Luau",2,3)
local sum = test(2.54,2.32);
print("C++ function called from Luau", sum)

local c = get_counter(1)
print(c, typeof(c))

c:increment()
c:increment()

print(c:get())

print(pcall(c.get, 42))

local M = {}

function M.damage(amount: number): number
	player.health = player.health - amount
	print("Luau: " .. describe(player))
	return player.health
end

function M.bump()
	c:increment()
end

return M
