-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Animated Sprite"
    },
    
    onPreload = function(self)
        -- load spritesheet with 32 x 32 pixel frames
        self.load:spritesheet("amara", "amara_tiny.png", 32, 32)

        -- Create animations for the "amara" texture
        self.animations:add({
            texture = "amara",
            key = "idle",
            startFrame = 1,
            numFrames = 8,
            frameRate = 6,
            loop = true
        })
        self.animations:add({
            texture = "amara",
            key = "run",
            startFrame = 9,
            endFrame = 16,
            frameRate = 12,
            loop = true
        })
    end,

    onCreate = function(self)
        -- Creating the sprite
        self.get.amara = self:createChild("Sprite", {
            texture = "amara",
            animation = "idle" -- Setting the initial animation
        })
    end,

    onUpdate = function(self, deltaTime)
        local anim = "idle"
        
        if Keyboard:isDown(Key.Left) and Keyboard:isDown(Key.Right) then
            anim = "idle"
        elseif Keyboard:isDown(Key.Left) then
            anim = "run"
            self.get.amara.scale = { -1, 1 } -- Flip in x-axis toface left.
        elseif Keyboard:isDown(Key.Right) then
            anim = "run"
            self.get.amara.scale = { 1, 1 }
        end

        self.get.amara.animation = anim
    end
})