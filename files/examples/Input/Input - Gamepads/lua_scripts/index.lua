-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Gamepad"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
        
        self.load:image("player", "player.png")
    end,

    onCreate = function(self)
        self.get.player = self:createChild("Sprite", {
            texture = "player"
        })

        self.get.gamepadNumberTxt = self:createChild("Text", {
            font = "defaultFont",
            text = "Gamepads Connected: 0",
            y = -32
        })

        -- Gamepad event callbacks
        Gamepads.onGamepadConnected = function()
            self.get.gamepadNumberTxt.text = "Gamepads Connected: " .. Gamepads.count
        end
        Gamepads.onGamepadDisconnected = function()
            self.get.gamepadNumberTxt.text = "Gamepads Connected: " .. Gamepads.count
        end
    end,

    onUpdate = function(self, deltaTime)
        if Gamepads.count > 0 then -- Check if any gamepads are connected.

            local gamepad = Gamepads:get(1) -- Get the first gamepad

            local speed = 100
            if gamepad:isDown(Button.RightTrigger) then
                speed = 300 * gamepad.rightTrigger -- Exact right trigger pressure number.
            end

            self.get.player:move(
                gamepad.leftStick.x * speed * deltaTime,
                gamepad.leftStick.y * speed * deltaTime
            )

            self.get.player:rotate(gamepad.rightStick.x * 100 * deltaTime)

            if not self.get.player.get.isJumping then
                if gamepad:isDown(Button.A) then
                    self.get.player.get.isJumping = true

                    self.get.player.tween:to({
                        z = -10,
                        duration = 0.2,
                        yoyo = true,
                        onComplete = function()
                            self.get.player.get.isJumping = false
                        end
                    })
                end
            end
        end
    end
})