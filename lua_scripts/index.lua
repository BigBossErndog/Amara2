Scripts:run("utility/localize")
Scripts:run("utility/project")

NodeFactory:load("UIWindow", "ui/UIWindow")
NodeFactory:load("UIButton", "ui/UIButton")
NodeFactory:load("ToolTips", "ui/ToolTips")
NodeFactory:load("TextField", "ui/TextField")

NodeFactory:load("MainWindow", "windows/MainWindow")
NodeFactory:load("NewProjectWindow", "windows/NewProjectWindow")

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
        world.load:image("toolTipBox", "ui/amara2_toolTipBox.png")
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16)
        world.load:spritesheet("uiIcons", "ui/amara2_icons.png", 16, 16)
        
        world.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        world.load:shaderProgram("gaussianHorizontal", {
            vertex = "defaultVert",
            fragment = "shaders/gaussianHorizontal.frag"
        })
        world.load:shaderProgram("gaussianVertical", {
            vertex = "defaultVert",
            fragment = "shaders/gaussianVertical.frag"
        })
        world.load:shaderProgram("boxBlur", {
            vertex = "defaultVert",
            fragment = "shaders/boxBlur.frag"
        })

        Localize:registerJSON(System:readJSON("data/localization/keywords.json"))
    end,
    onCreate = function(world)
        local props = world.props;

        props.windowShadows = world:createChild("ShaderContainer", {
            alpha = 0.75,
            tint = Colors.Black,
            repeats = 3,
            shaderPasses = { "boxBlur" },
            onCreate = function(self)
                self.size = self.world.view
            end,
            onUpdate = function(self)
                self.size = self.world.view
            end
        })

        props.windows = world:createChild("Group")

        props.mainwin = props.windows:createChild("MainWindow")
        props.mainwin.func:openWindow()

        props.windowShadows_copy = props.windowShadows:createChild("CopyNode", {
            target = props.windows,
            x = -6, y = 8
        })

        props.toolTips = world:createChild("ToolTips")
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