return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        transparent = true,
        clickThrough = true,
        alwaysOnTop = true,
        vsync = true,
        screenMode = ScreenMode.BorderlessWindowed,
        graphics = Graphics.OpenGL
    },
    onCreate = function(world)
        Game:uncapFPS()
        world:fitToDisplay()

        -- world.clickThrough = false

        local rect = world:createChild("FillRect", {
            width = 128,
            height = 128,
            color = "red",
            input = true
        })
        rect.input:listen("onPointerHover", function(self)
            
        end)
        rect.input:listen("onPointerUp", function(self)
            self.tween:from({
                rotation = 0
            }):to({
                rotation = 2*math.pi,
                duration = 2,
                ease = Ease.SineInOut
            })
        end)
    end,
    onUpdate = function(world)
        -- world.clickThrough = false
    end
})