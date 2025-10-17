Nodes:define("PlatformerScene", "Scene", {
    onCreate = function(self)
        -- Creating controls 

        local leftControl = Controls:scheme("left")
        leftControl:setKeys(Key.Left, Key.A)
        leftControl:setButtons(Button.DpadLeft, Button.LeftStickLeft)

        local rightControl = Controls:scheme("right")
        rightControl:setKeys(Key.Right, Key.D)
        rightControl:setButtons(Button.DpadRight, Button.LeftStickRight)

        local upControl = Controls:scheme("up")
        upControl:setKeys(Key.Up, Key.W)
        upControl:setButtons(Button.DpadUp, Button.LeftStickUp, Button.B)
        
        -- Creating a platform using a FillRect
        -- You can make the platform out of anything: Text, TilemapLayer, etc.
        local platform = self:createChild("FillRect", {
            color = Colors.White,
            width = 128,
            height = 24,
            y = 64
        })

        local player = self:createChild("FillRect", {
            color = Colors.White,
            width = 20,
            height = 28,

            -- You can create a collider for any node: Sprite, FillRect, Group, etc.
            collider = {
                target = platform, -- Your target can be any node.
                acceleration = { 0, 800 }, -- Acceleration for gravity.
                damping = { 0.998, 0 } -- Damping slows velocity, here slows the x velocity.
            },

            onUpdate = function(self)
                local control = 0 -- for controlling left/right movement. Prevent moving if left and right are both pressed.

                if Controls:isDown("up") and self.collider:hasCollided(Direction.Down) then
                    self.collider.velocity.y = -300
                end

                if Controls:isDown("left") then
                    control = control - 120
                end
                if Controls:isDown("right") then
                    control = control + 120
                end

                self.collider.velocity.x = control
            end
        })

        -- Making the scene's camera follow the player.
        self.camera.followTarget = player
    end
})

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "2D Platformer Collision"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
    end,

    onCreate = function(self)
        -- Creating the scene
        self:createChild("PlatformerScene")
    end,

    onUpdate = function(self, deltaTime)

    end
})