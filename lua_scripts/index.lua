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

        -- props.windowShadows = world:createChild("ShaderContainer", {
        --     tint = Colors.Black,
        --     onCreate = function(self)
        --         print("WHAT THE FUCK")
        --         print(1, self) 
        --         self.size = self.world.view
        --     end,
        --     onUpdate = function(self)
        --         print(2, self)
        --         self.size = self.world.view
        --     end
        -- })

        props.windows = world:createChild("Group")

        props.mainwin = props.windows:createChild("MainWindow")
        props.mainwin.height = 0
        props.mainwin.func:openBox(64, function(win)
            win.props.content.visible = true
        end)

        -- print(props.windowShadows)
        -- props.windowShadows_copy = props.windowShadows:createChild("CopyNode", {
        --     target = props.windows,
        --     x = -24, y = 24
        -- })
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