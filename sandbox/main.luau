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
print("Luau: " .. describe(player))

local M = {}

function M.damage(amount: number): number
	player.health = player.health - amount
	print("Luau: " .. describe(player))
	return player.health
end

return M
