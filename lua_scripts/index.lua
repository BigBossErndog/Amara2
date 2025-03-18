function print_all_fields(obj)
    local mt = getmetatable(obj)
    if mt then
        -- Check if __index exists in the metatable (this is where methods/properties usually reside)
        if mt.__index then
            print("== __index ==")
            for key, value in pairs(mt.__index) do
                print(key, value)
            end
        else
            print("== Metatable ==")
            for key, value in pairs(mt) do
                print(key, value)
            end
        end
    end
end

function printObjectInfo(obj)
    print("Object properties and methods:")
    
    -- Iterate through the object's fields and methods
    for key, value in pairs(obj) do
        local valueType = type(value)
        if valueType == "function" then
            print(key .. " (method)")
        else
            print(key .. " (property: " .. valueType .. ")")
        end
    end
end
-- scripts:run("scripts/testScript.lua");

-- game:setTargetFPS(5)


local w = creator:createWorld()

local e = w:createChild("Entity")
e.id = "acting entity"

local a = e:createChild("Action")
local t = e:createChild("Tween"):to({
    x = 3,
    duration = 3,
    onFinish = function(self)
        self:createChild("Tween"):to({
            x = 0,
            duration = 3
        })
    end
})
e.onUpdate = function(self, delta)
    log(e.x, " ", game.fps)
end