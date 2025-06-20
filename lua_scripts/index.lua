Scripts:run("utility/localize")

NodeFactory:load("UIWindow", "ui/UIWindow")
NodeFactory:load("MainWindow", "windows/MainWindow")

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        transparent = true,
        screenMode = ScreenMode.BorderlessFullscreen,
        clickThrough = true,
        alwaysOnTop = true,
        vsync = true,
        graphics = Graphics.OpenGL
    },
    onPreload = function(world) 
        world:fitToDisplay()
        world.load:image("uiBox", "ui/amara2_uiBox.png")
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16)
        world.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
    end,
    onCreate = function(world)
        local props = world.props;

        props.tc = world:createChild("TextureContainer", {
            fill = { 0, 0, 0, 80 },
            width = 1920,
            height = 1080,
            onCreate = function(self)
                self.size = self.world.view
            end,
            onUpdate = function(self)
                self.size = self.world.view
            end
        })

        props.tc:createChild("TextureContainer", {
            fill = Colors.Black,
            width = 64,
            height = 64
        })
        -- props.tc.size = world.view
        -- print(world.view)
        -- props.tc.x = 0
        -- props.tc.y = 0
        -- print(props.tc.rect)

        -- props.tc:createChild("Sprite", {
        --     texture = "uiBox",
        --     onUpdate = function(self)
        --         if Keyboard:isDown(Key.Left) then
        --             self.x = self.x - 1
        --         end
        --         if Keyboard:isDown(Key.Right) then
        --             self.x = self.x + 1
        --         end
        --         if Keyboard:isDown(Key.Up) then
        --             self.y = self.y - 1
        --         end
        --         if Keyboard:isDown(Key.Down) then
        --             self.y = self.y + 1
        --         end
        --     end
        -- })

        -- props.windowShadows = props.tc:createChild("TextureContainer", {
        --     fill = Colors.Black,
        --     onCreate = function(self)
        --         self.size = self.world.view
        --         -- 
        --     end,
        --     onUpdate = function(self)
        --         self.size = self.world.view
        --     end
        -- })

        props.windows = world:createChild("Group")

        props.sprite = props.windows:createChild("Sprite", {
            texture = "uiBox",
            maxWidth = 640, maxHeight = 640,
            input = {
                active = true,
                draggable = true
            }
        })

        props.mainwin = props.windows:createChild("MainWindow")
        props.mainwin.height = 0
        props.mainwin.func:openBox(64, function(win)
            win.props.content.visible = true
        end)

        props.windowShadows_copy = props.tc:createChild("CopyNode", {
            target = props.windows,
            alpha = 0.5,
            x = -4, y = 4,
            depth = -1
        })
    end,
    onUpdate = function(world, deltaTime)
        if Keyboard:justPressed(Key.One) then
            world:fitToDisplay(1)
        end
        if Keyboard:justPressed(Key.Two) then
            world:fitToDisplay(2)
        end
        if Keyboard:justPressed(Key.Three) then
            world:fitToDisplay(3)
        end
        if Keyboard:justPressed(Key.Four) then
            world:fitToDisplay(4)
        end
    end
})