NodeFactory:add("Test", function()
    local e = NodeFactory:create("Node")

    e.onCreate = function(self)
        print(self, "was created!")
    end
    e.bind.hello = function(self, msg)
        print("Hello from", self, "!")
    end

    return e
end)


local e = NodeFactory:create("Test")
-- e.props.name = "hi"


local f = e:createChild("Test")
f.id = "entity 2"
e.id = "entity 1"

e.pos.x = e.pos.x + 1
print(e.id)
e.call:hello()
f.call:hello()

if Game.arguments then
    log(Game.arguments.get(1))
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
local b = p:starts_with("jfk")
print(b)

local d = e.children:find("entity 2");
-- w:start()

print(string.concat(
    "1", "2", "3"
))

print(d)

-- files:execute("start ms-settings:defaultapps")
-- local c = e:createChild("Scene")

-- local contents = files:getDirectoryContents("lua_scripts")
-- for i, v in ipairs(contents) do
--     print(v)
-- end