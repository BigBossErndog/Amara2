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
        local win = world:createChild("MainWindow")
        -- win.height = 0
        -- print(win.call)
        -- win.call:openBox(64)
        win.call:openBox(64, function(win)
            win.props.content.visible = true
        end)

        world.props.win = win
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