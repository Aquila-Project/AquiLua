--!strict

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
