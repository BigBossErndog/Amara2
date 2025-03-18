function print_all_fields(obj)
    print("== Properties ==")

    -- Check if obj is a table (Lua table or userdata)
    if type(obj) == "table" then
        for key, value in pairs(obj) do
            print(key, value)
        end
    end

    -- Get the metatable
    local mt = getmetatable(obj)
    if mt then
        -- Check if __index exists in the metatable (this is where methods/properties usually reside)
        if mt.__index then
            if type(mt.__index) == "table" then
                print("== Methods in __index (sol::table) ==")
                for key, value in pairs(mt.__index) do
                    print(key, value)
                end
            elseif type(mt.__index) == "function" then
                print("== Method in __index (function) ==")
                print(mt.__index)  -- just print the function reference
            else
                print("== Metatable ==")
                for key, value in pairs(mt) do
                    print(key, value)
                end
            end
        else
            print("== Metatable ==")
            for key, value in pairs(mt) do
                print(key, value)
            end
        end
    end
end
-- scripts:run("scripts/testScript.lua");

game:setTargetFPS(5)


local w = creator:createWorld()
local e = w:createChild("Entity")

local t = e:createChild("Tween")
-- log(t)
local s = Scene.new()
print(s)
print_all_fields(e)

-- t:to({ x = 10, y = 2 })
-- e.onUpdate = function(self, delta)
--     log(self.pos)
-- end