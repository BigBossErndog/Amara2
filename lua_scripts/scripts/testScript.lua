game.factory:add("TestEntity", "entities/testEntity")
local e = game.factory:create("TestEntity")
local f = e:createChild("TestEntity")

if game.arguments then log(game.arguments) end

-- e.pos = Vector3.new(1, 2, 3)
print(e.props.hi)
log(e.pos:subtract(1, 2))

-- if game.files:compile("lua_scripts/entities/testEntity.lua", "lua_scripts/entities/testEntity.luac") then
--     game.files:deleteFile("lua_scripts/entities/testEntity.lua")
-- end

-- d.props:hello()
-- game:execute("make win64")

-- e.props.wtf("hello")
-- print(f.id)
-- local c = e:createChild("Scene")



-- local contents = files:getDirectoryContents("lua")
-- for i, v in ipairs(contents) do
--     print(v)
-- end