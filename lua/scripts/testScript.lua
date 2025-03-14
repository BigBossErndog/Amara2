factory:add("testEntity", "entities/testEntity.lua")
local e = factory:create("testEntity")
print(e.id)
local contents = files:getDirectoryContents("lua")
for i, v in ipairs(contents) do
    print(v)
end