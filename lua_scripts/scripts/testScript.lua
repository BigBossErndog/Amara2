game.factory:add("TestEntity", "entities/testEntity.lua")
local e = game.factory:create("TestEntity")
e.id = "testEntity1"
local f = e:createChild("TestEntity")
f.id = "testEntity2"

log("Arguments: ", game.arguments);

-- e.pos = Vector3.new(1, 2, 3)
log(e.pos)
e.pos:add(1, 2)
log(e.pos)

-- d.props:hello()
-- game:execute("make win64")

-- e.props.wtf("hello")
-- print(f.id)
-- local c = e:createChild("Scene")



-- local contents = files:getDirectoryContents("lua")
-- for i, v in ipairs(contents) do
--     print(v)
-- end