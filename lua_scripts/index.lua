local uiBox;

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        transparent = true,
        clickThrough = true,
        alwaysOnTop = true,
        graphics = Graphics.OpenGL
    },
    onPreload = function(world) 
        world:fitToDisplay()
        world.load:image("uiBox", "ui/amara2_uiBox.png");
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16);
    end,
    onCreate = function(world)
        for i = 1, 10 do
            uiBox = world:createChild("NineSlice", {
                texture = "uiBox",
                x = math.random() * world.vw - world.vw/2,
                y = math.random() * world.vh - world.vh/2,
                maxWidth = 640, maxHeight = 640,
                width = 64, height = 64,
                input = true
            })
            uiBox.input.draggable = true

            uiBox.input:listen("onPointerUp", function(self, pointer)
                if pointer.state.timeHeld < 0.2 then
                    self.tween:from({
                        rotation = 0
                    }):to({
                        rotation = math.pi,
                        duration = 1,
                        ease = Ease.SineInout
                    })
                end
            end)
        end
    end
})