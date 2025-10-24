-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Clicking Buttons"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        self.load:spritesheet("yesNo", "yesNo.png", 32, 24)
    end,

    onCreate = function(self)
        self.get.button = self:createChild("Sprite", {
            texture = "yesNo",

            input = {
                active = true, -- Must be set to true.

                cursor = Cursor.Pointer, -- Change the mouse cursor to a pointing cursor, helps suggest node is clickable when hovering.

                onPointerDown = function(self) -- 'self' now refers to the button
                    if self.frame == 1 then
                        self.frame = 2
                    else
                        self.frame = 1
                    end
                end,

                -- Here are some other events:
                -- onPointerUp - When a mouse or finger is released while hovering.

                -- onPointerHover - When a mouse or finger enters the node's space.
                -- onPointerExit - When a mouse or finger leaves the node's space.

                -- If you specifically want to use mouse (no touch):
                -- onMouseDown, onMouseUp, etc

                -- If you specifically want touch
                -- onTouchDown, onTouchUp, etc

                -- Left, Right, Middle mouse clicks.
                -- onLeftMouseDown, onRightMouseDown, onMiddleMouseDown
            }
        })

        -- You can activate or deactivate with:
        -- self.get.button.input = true/false
    end,

    onUpdate = function(self, deltaTime)

    end
})