world.factory:add("TestEntity", "entities/testEntity")
local e = world.factory:create("TestEntity")
e.id = "1"
-- e.props.name = "hi"
log(e.props)


local f = e:createChild("TestEntity")
f.id = "2"

e.props.hello("HI!")

if world.arguments then log(world.arguments) end

-- e.pos = Vector3.new(1, 2, 3)
-- print(e.props.hi)
-- log(e.pos:subtract(1, 2))

-- function buildScripts(currentPath, currentRoute)
--     local subdirs, files;
--     files = world.files:getFilesInDirectory(currentPath)
--     for i, v in ipairs(files) do
--         if (v:ends_with(".lua")) then
--             local d = world.files:getRelativePath(currentRoute) .. "/" .. world.files:removeFileExtension(
--                 world.files:getFileName(v)
--             ) .. ".luac"
--             world.files:compileScript(v, d)
--         end
--     end
--     subdirs = world.files:getSubDirectories(currentPath)
--     if #subdirs > 0  then
--         for i, v in ipairs(subdirs) do
--             buildScripts(v, currentRoute .. "/" .. world.files:getDirectoryName(v))
--         end
--     end
-- end

-- buildScripts("./lua_scripts", "./build/lua_scripts")

-- world.files:copy("Makefile", "MakeFile.txt")

local p = "hello world"
print(p:starts_with("hello"))

local d = e.children:get(1)
-- log(d.props:hello())

-- local c = e:createChild("Scene")

-- local contents = world.files:getDirectoryContents("lua_scripts")
-- for i, v in ipairs(contents) do
--     print(v)
-- end