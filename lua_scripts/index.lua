-- game:uncapFPS()

-- local e = factory:create("Node");
-- e.props.hello = function() log("hi") end

NodeFactory:create("Node")

local firstworld = Creator:createWorld()
firstworld:configure({
    window = {
        windowTitle = "Amara2",
        resizable = true,
        -- width = 1280,
        -- height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        graphics = Graphics.Render2D
    }
})

-- Every window is represented with a World.
-- You can create multiple worlds.
-- Calling world:destroy() will close the window.

local e = firstworld:createChild("Node")

-- -- e.id = "acting entity"
-- world.onUpdate = function(self, deltaTime)
--     print(deltaTime)
-- end
-- w.id = "hi"
-- e.onUpdate = function(self) log(world.x) end

-- print_metatable(w)

-- local s = world:createChild("Scene")
-- local g = world:createChild("Scene")

-- local a = world:createChild("Action")

-- assert(false, "IT FAILED!!!")

Scripts:load("test","scripts/testScript")
Scripts:run("test")

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
-- Game:setTargetFPS(30)
firstworld.onPreload = function(self)
    self.load:image("goldenFlower", "goldenFlower.png")
end
firstworld.onCreate = function(self)
    self.load:whenDone(function(self)
        local s = self:createChild("Sprite")
        s:loadTexture("goldenFlower")
        s.id = "test"

        s.scale.x = 1
        s.scale.y = 1

        -- s.cropLeft = 70

        local t = s:createChild("Sprite")
        t:loadTexture("goldenFlower")
        t.x = t.x + 140
        t.rotation = math.rad(90)
        -- t.cropLeft = 70

        local u = t:createChild("Sprite")
        u:loadTexture("goldenFlower")
        u.x = u.x + 140
        -- u.cropLeft = 70

        self.props.s = s;
    end)
end

firstworld.onUpdate = function (self, deltaTime)
    if self.props.s then self.props.s:rotate(2 * math.pi * deltaTime * 0.25) end
end

-- function tweenWorld(w)
--     w.onCreate = function(world
--         world.tween:from({
--                 x = (world.display.w - world.w)/2
--             }):to({
--                 duration = 1
--             }):to({
--                 x = world.display.w - world.w,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 x = 0,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 x = (world.display.w - world.w)/2,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 y = 0,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 y = (world.display.h - world.h)/2,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 x = (world.display.w - 640*2)/2,
--                 y = (world.display.h - 360*2)/2,
--                 w = 640*2,
--                 h = 360*2,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):to({
--                 x = (world.display.w - 640)/2,
--                 y = (world.display.h - 360)/2,
--                 w = 640,
--                 h = 360,
--                 duration = 1,
--                 ease = Ease.SineInOut
--             }):whenDone(function(self)  
--                 -- print("finished")
--                 -- self.world:destroy()
--                 -- local newworld = Creator:createWorld()
--                 -- newworld:configure({
--                 --     window = {
--                 --         windowTitle = "Amara2",
--                 --         -- resizable = true
--                 --         -- width = 1280,
--                 --         -- height = 720
--                 --     }
--                 -- })
--                 -- tweenWorld(newworld)
--             end)
--         end
--     end
-- tweenWorld(firstworld)