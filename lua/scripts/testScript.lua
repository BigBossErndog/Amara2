factory:add("testEntity", "entities/testEntity.lua")
local e = factory:create("scene")
print(e.entityID)
local contents = files:getDirectoryContents("lua")
for i, v in ipairs(contents) do
    print(v)
end