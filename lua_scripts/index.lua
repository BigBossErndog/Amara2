local rect;

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

        rect = world:createChild("FillRect", {
            width = 128,
            height = 128,
            color = "red",
            input = true
        })
        rect.input.draggable = true
        -- rect.input:listen("onPointerHover", function(self)
            
        -- end)
        rect.props.count = 0
        rect.input:listen("onTouchUp", function(self)
            print("FINGER!")
        end)
        rect.input:listen("onTouchUp", function(self, pointer)
            if pointer.state.timeHeld < 0.2 then
                rect.props.count = rect.props.count + 1
                print(rect.props.count)

                self.tween:from({
                    rotation = 0
                }):to({
                    rotation = math.pi,
                    duration = 1,
                    ease = Ease.SineInOut
                })
            end
        end)
    end,
    onUpdate = function(world)
        if Keyboard:justPressed(Key.One) then
            world.tween:to({
                rect = Game:getDisplayBounds(1),
                duration = 0.2,
                ease = Ease.SineInOut
            })
        end
        if Keyboard:justPressed(Key.Two) then
            world.tween:to({
                rect = Game:getDisplayBounds(2),
                duration = 0.2,
                ease = Ease.SineInOut
            })
        end
        if Keyboard:justPressed(Key.Three) then
            world.tween:to({
                rect = Game:getDisplayBounds(3),
                duration = 0.2,
                ease = Ease.SineInOut
            })
        end
        if Keyboard:justPressed(Key.Four) then
            world.tween:to({
                rect = Game:getDisplayBounds(4),
                duration = 0.2,
                ease = Ease.SineInOut
            })
        end
    end
})