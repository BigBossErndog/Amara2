-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Sound Effects"
    },
    
    onPreload = function(self)
        self.load:image("soundButton", "soundButton.png")

        -- Load audio. wav, ogg, mp3
        self.load:audio("jump", "sounds/jump.wav")
        self.load:audio("lose", "sounds/lose.wav")
        self.load:audio("coin", "sounds/coin.wav")
    end,
    
    createAudio = function(self)
        -- Create your Audio Nodes.
        self.audio:createChild("Audio", {
            audio = "jump"
        })
        self.audio:createChild("Audio", {
            audio = "lose"
        })
        self.audio:createChild("Audio", {
            audio = "coin"
        })
    end,

    onCreate = function(self)
        -- Calling the custom function.
        self.func:createAudio()

        self:createChild("Sprite", {
            texture = "soundButton",
            x = -64,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(self)
                    -- Tween to animate button press
                    self.scale = 0.9
                    self.tween:to({
                        scale = 1,
                        duration = 0.2,
                        ease = Ease.BackOut
                    })

                    -- Play your audio sounds.
                    self.audio:play("jump")
                end
            }
        })

        self:createChild("Sprite", {
            texture = "soundButton",
            x = 0,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(self)
                    -- Tween to animate button press
                    self.scale = 0.9
                    self.tween:to({
                        scale = 1,
                        duration = 0.2,
                        ease = Ease.BackOut
                    })
                    self.audio:play("lose")
                end
            }
        })

        self:createChild("Sprite", {
            texture = "soundButton",
            x = 64,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(self)
                    -- Tween to animate button press
                    self.scale = 0.9
                    self.tween:to({
                        scale = 1,
                        duration = 0.2,
                        ease = Ease.BackOut
                    })
                    self.audio:play("coin")
                end
            }
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})