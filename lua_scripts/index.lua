-- game:uncapFPS()

-- local e = factory:create("Entity");
-- e.props.hello = function() log("hi") end
local world = Creator:createWorld()

world:configure({
    window = {
        windowTitle = "Amara2",
        width = 1280,
        height = 720,
        graphics = Graphics.Render2D
    }
})

local world = Creator:createWorld()

world:configure({
    window = {
        windowTitle = "Amara2_B",
        width = 1280,
        height = 720,
        graphics = Graphics.Render2D
    }
})
-- Every window is represented with a World.
-- You can create multiple worlds.
-- Calling world:destroy() will close the window.

Game:setTargetFPS(30)

local e = world:createChild("Entity")

-- -- e.id = "acting entity"
-- world.onUpdate = function(self, deltaTime)
--     print(deltaTime)
-- end
-- w.id = "hi"
-- e.onUpdate = function(self) log(world.x) end

-- print_metatable(w)

local s = world:createChild("Scene")

-- e.tween:from({
--         x = 10
--     }):to({
--         x = 0,
--         duration = 1,
--         onUpdate = function(self)
--             print(e.x)
--         end,
--         ease = Ease.SineOut
--     }):to({
--         x = 10,
--         duration = 1,
--         onUpdate = function(self)
--             print(e.x)
--         end
--     }):whenDone(function(self)
--         print("finished")
--     end)

-- Scripts:run("scripts/testScript")

-- Return the world if you want Amara to manage it and give you debugging tools.
return world