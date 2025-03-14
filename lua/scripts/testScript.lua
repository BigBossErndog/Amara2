factory:add("TestEntity", "entities/testEntity.lua")
local e = factory:create("TestEntity")
e.id = "testEntity1"
local f = factory:create("TestEntity")
f.id = "testEntity2"
local c = e:createChild("Scene");

e.props.hello()

-- game:execute("make win64")

-- e.props.wtf("hello")
-- print(f.id)
-- local c = e:createChild("Scene")



-- local contents = files:getDirectoryContents("lua")
-- for i, v in ipairs(contents) do
--     print(v)
-- end