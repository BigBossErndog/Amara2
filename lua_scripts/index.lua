-- game:uncapFPS()

-- local e = factory:create("Node");
-- e.props.hello = function() log("hi") end


-- Every window is represented with a World.
-- You can create multiple worlds.
-- Calling world:destroy() will close the window.

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

-- Scripts:load("test","scripts/testScript")
-- Scripts:run("test")

    -- Game:setTargetFPS(60)
    -- Creator:createWorld({
    --     window = {
    --         windowTitle = "Amara2",
    --         -- resizable = true,
    --         screenMode = ScreenMode.Windowed,
    --         width = 1280,
    --         height = 720,
    --         virtualWidth = 640,
    --         virtualHeight = 360,
    --         graphics = Graphics.OpenGL
    --         -- graphics = Graphics.Render2D,
    --     },
    --     entryScene = "scenes/TestScene",
    -- })
    
    Creator:createWorld({
        window = {
            windowTitle = "Amara2",
            -- resizable = true,
            screenMode = ScreenMode.Windowed,
            width = 1280,
            height = 720,
            virtualWidth = 640,
            virtualHeight = 360,
            graphics = Graphics.OpenGL
            -- graphics = Graphics.Render2D,
        },
        entryScene = "scenes/TestScene",
        -- onPreload = function(self)
        --     self.load:image("goldenFlower", "goldenFlower.png")
        -- end,
        -- onCreate = function(self)
        --     local textCont = self:createChild("TextureContainer", {
        --         width = 256,
        --         height = 256,
        --         alpha = 0.5,
        --         -- visible = false,
        --         paused = true,
        --         canvasLocked = true
        --     })
    
        --     local a_rate = 2 * math.pi * 0.01
        --     local d_rate = 1
        --     for i = 1, (128*128) do
        --         textCont:createChild("Sprite", {
        --             texture = "goldenFlower",
        --             x = math.sin(a_rate * i) * d_rate * i,
        --             y = math.cos(a_rate * i) * d_rate * i
        --         })
        --     end
        -- end,
        onUpdate = function(self, deltaTime)
            -- print(Game.fps)
        end
    })

--     firstworld.onPreload = function(self)
--         self.load:image("goldenFlower", "goldenFlower.png")
--         self.load:spritesheet("freaker", "freaker.png", 32, 64);
        
--         Animations:add({
--             key = "running",
--             texture = "freaker",
--             startFrame = 3,
--             numFrames = 6,
--             frameRate = 12,
--             repeats = -1
--         })
--     end
--     firstworld.onCreate = function(self)
--         -- print(self.w, self.h)
--         -- print(self.display.w, self.display.h)

--         local s = self:createChild("Sprite")
--         s:setTexture("goldenFlower")
--         s.id = "test"

--         s.scale.x = 2
--         s.scale.y = 2

--         -- s.cropLeft = 70

--         local t = s:createChild("Sprite")
--         t:setTexture("goldenFlower")
--         t.x = t.x + 140
--         t.rotation = math.rad(90)
--         -- t.cropLeft = 70

--         local u = t:createChild("Sprite")
--         u:setTexture("goldenFlower")
--         u.x = u.x + 140
--         -- u.cropLeft = 70

--         self.props.s = s;

--         local f = self:createChild("Sprite")
--         f:setTexture("freaker")
--         f:animate("running")
--         self.props.f = f;
--         f.frame = 3

--         local controlRight = Controls:scheme("right");
--         controlRight:setKeys({ Key.Right, Key.D });

--         local controlLeft = Controls:scheme("left");
--         controlLeft:setKeys({ Key.Left, Key.A });

--         local controlUp = Controls:scheme("up");
--         controlUp:setKeys({ Key.Up, Key.W });

--         local controlDown = Controls:scheme("down");
--         controlDown:setKeys({ Key.Down, Key.S });

--         -- tweenWorld(self)
--     end

--     firstworld.onUpdate = function (self, deltaTime)
--         

--     end

-- function tweenWorld(world)
--     world.tween:from({
--             x = (world.display.w - world.w)/2
--         }):to({
--             duration = 1
--         }):to({
--             x = world.display.w - world.w,
--             duration = 1,
--             ease = Ease.SineInOut
--         }):to({
--             x = 0,
--             duration = 1,
--             ease = Ease.SineInOut
--         }):to({
--             x = (world.display.w - world.w)/2,
--             duration = 1,
--             ease = Ease.SineInOut
--         }):to({
--             y = 0,
--             duration = 1,
--             ease = Ease.SineInOut
--         }):to({
--             y = (world.display.h - world.h)/2,
--             duration = 1,
--             ease = Ease.SineInOut
--         }):whenDone(function(this)
--             this.world:setScreenMode(ScreenMode.Fullscreen)
--         end)
-- end