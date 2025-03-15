game.factory:add("TestEntity", "entities/testEntity")
local e = game.factory:create("TestEntity")
e.id = "testEntity1"
local f = e:createChild("TestEntity")
f.id = "testEntity2"

if game.arguments then log(game.arguments) end

-- e.pos = Vector3.new(1, 2, 3)
log(e.pos:subtract(1, 2))

-- d.props:hello()
-- game:execute("make win64")

-- e.props.wtf("hello")
-- print(f.id)
-- local c = e:createChild("Scene")



-- local contents = files:getDirectoryContents("lua")
-- for i, v in ipairs(contents) do
--     print(v)
-- end