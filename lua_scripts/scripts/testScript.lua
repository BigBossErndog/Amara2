factory:load("TestEntity", "entities/testEntity")

local e = factory:create("TestEntity")
print(e)
-- e.props.name = "hi"


local f = e:createChild("TestEntity")
f.id = "entity 2"
e.id = "entity 1"

e.pos.x = e.pos.x + 1
log(e.id)
e:hello()
f:hello()

if game.arguments then
    log(game.arguments(1))
end

-- log(game.platform)


-- e.pos = Vector3.new(1, 2, 3)
-- print(e.props.hi)
-- log(e.pos:subtract(1, 2))

-- function buildScripts(currentPath, currentRoute)
--     local subdirs, files;
--     files = files:getFilesInDirectory(currentPath)
--     for i, v in ipairs(files) do
--         if (v:ends_with(".lua")) then
--             local d = files:getRelativePath(currentRoute) .. "/" .. files:removeFileExtension(
--                 files:getFileName(v)
--             ) .. ".luac"
--             files:compileScript(v, d)
--         end
--     end
--     subdirs = files:getSubDirectories(currentPath)
--     if #subdirs > 0  then
--         for i, v in ipairs(subdirs) do
--             buildScripts(v, currentRoute .. "/" .. files:getDirectoryName(v))
--         end
--     end
-- end

-- buildScripts("./lua_scripts", "./build/lua_scripts")

-- files:copy("Makefile", "MakeFile.txt")

local p = "hello world"
print(p:starts_with("hello"))

local d = e.children:find("entity2");
-- print(w)
-- w:start()

log(string.concat(
    "1", "2", "3"
))

-- files:execute("start ms-settings:defaultapps")
-- local c = e:createChild("Scene")

-- local contents = files:getDirectoryContents("lua_scripts")
-- for i, v in ipairs(contents) do
--     print(v)
-- end