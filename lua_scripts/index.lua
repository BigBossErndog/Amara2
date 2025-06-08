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
        graphics = Graphics.Render2D
    },
    onPreload = function(world) 
        world:fitToDisplay()
        world.load:image("uiBox", "ui/amara2_uiBox.png");
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16);
    end,
    onCreate = function(world)
        for i = 1, 10 do
            world:createChild("ui/UIWindow", {
                x = math.random() * world.vw - world.vw/2,
                y = math.random() * world.vh - world.vh/2
            })
        end
    end,
    onUpdate = function(world)
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