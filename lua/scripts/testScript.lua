factory:add("TestEntity", "entities/testEntity.lua")
local e = factory:create("TestEntity")
e.id = "1"
local f = factory:create("TestEntity")
f.id = "2"
local c = e:createChild("Scene");

log("wtf ", "is ", "dis")

-- game:execute("make win64")

-- e.props.wtf("hello")
-- print(f.id)
-- local c = e:createChild("Scene")



-- local contents = files:getDirectoryContents("lua")
-- for i, v in ipairs(contents) do
--     print(v)
-- end