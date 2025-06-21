Scripts:run("utility/localize")

NodeFactory:load("UIWindow", "ui/UIWindow")
NodeFactory:load("MainWindow", "windows/MainWindow")

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        -- backgroundColor = Colors.White,
        screenMode = ScreenMode.BorderlessFullscreen,
        transparent = true,
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

        world.load:shaderProgram("gaussianHorizontal", {
            vertex = "defaultVert",
            fragment = "shaders/gaussianHorizontal.frag"
        })
        world.load:shaderProgram("gaussianVertical", {
            vertex = "defaultVert",
            fragment = "shaders/gaussianVertical.frag"
        })
    end,
    onCreate = function(world)
        local props = world.props;

        props.windowShadows = world:createChild("ShaderContainer", {
            alpha = 0.75,
            tint = Colors.Black,
            repeats = 2,
            shaderPasses = { "gaussianHorizontal", "gaussianVertical" },
            onCreate = function(self)
                self.size = self.world.view
            end,
            onUpdate = function(self)
                self.size = self.world.view
            end
        })

        props.windows = world:createChild("Group")

        props.mainwin = props.windows:createChild("MainWindow")
        props.mainwin.height = 0
        props.mainwin.func:openBox(64, function(win)
            win.props.content.visible = true
        end)

        props.windowShadows_copy = props.windowShadows:createChild("CopyNode", {
            target = props.windows,
            x = -8, y = 8
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