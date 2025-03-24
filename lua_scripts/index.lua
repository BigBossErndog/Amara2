-- game:uncapFPS()

-- local e = factory:create("Node");
-- e.props.hello = function() log("hi") end

Factory:create("Node")

local world = Creator:createWorld()
world:configure({
    window = {
        windowTitle = "Amara2",
        resizable = true
        -- width = 1280,
        -- height = 720
    }
})

-- Every window is represented with a World.
-- You can create multiple worlds.
-- Calling world:destroy() will close the window.

local e = world:createChild("Node")

-- -- e.id = "acting entity"
-- world.onUpdate = function(self, deltaTime)
--     print(deltaTime)
-- end
-- w.id = "hi"
-- e.onUpdate = function(self) log(world.x) end

-- print_metatable(w)

local s = world:createChild("Scene")
local g = world:createChild("Scene")

local a = world:createChild("Action")

-- assert(false, "IT FAILED!!!")

Scripts:load("test","scripts/testScript")
-- Scripts:run("test")

-- world.onCreate = function(self)
--     world.tween:to({
--         x = (world.display.w - 640*2)/2,
--         y = (world.display.h - 360*2)/2,
--         w = 640*2,
--         h = 360*2,
--         duration = 1,
--         yoyo = true,
--         ease = Ease.SineInOut,
--         repeats = -1
--     })
-- end


world.onCreate = function(world)
    world.tween:from({
            x = (world.display.w - world.w)/2
        }):to({
            duration = 1
        }):to({
            x = world.display.w - world.w,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            x = 0,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            x = (world.display.w - world.w)/2,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            y = -world.h/2,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            y = (world.display.h - world.h)/2,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            x = (world.display.w - 640*2)/2,
            y = (world.display.h - 360*2)/2,
            w = 640*2,
            h = 360*2,
            duration = 1,
            ease = Ease.SineInOut
        }):to({
            x = (world.display.w - 640)/2,
            y = (world.display.h - 360)/2,
            w = 640,
            h = 360,
            duration = 1,
            ease = Ease.SineInOut
        }):whenDone(function(self)  
            print("finished")
        end)
    end

-- Return the world if you want Amara to manage it and give you debugging tools.
return world