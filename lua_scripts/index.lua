-- game:uncapFPS()

-- local e = factory:create("Entity");
-- e.props.hello = function() log("hi") end
local world = Creator:createWorld()
world:configure({
    window = {
        windowTitle = "Amara2",
        -- width = 1280,
        -- height = 720
    }
})

world = Creator:createWorld()
world:configure({
    window = {
        windowTitle = "Amara2_B",
        -- width = 1280,
        -- height = 720
    }
})

world = Creator:createWorld()
world:configure({
    window = {
        windowTitle = "Amara2_B",
        -- width = 1280,
        -- height = 720
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

world.tween:from({
        x = 0
    }):to({
        x = 1920-640,
        duration = 1,
        ease = Ease.SineInOut,
        onUpdate = function(self)
            print(self.x)
        end
    }):to({
        x = 0,
        duration = 1,
        ease = Ease.SineInOut,
        onUpdate = function(self)
            print(self.x)
        end
    }):to({
        x = (1920-640)/2,
        duration = 1,
        ease = Ease.SineInOut,
        onUpdate = function(self)
            print(self.x)
        end
    })

-- Scripts:run("scripts/testScript")

-- Return the world if you want Amara to manage it and give you debugging tools.
return world