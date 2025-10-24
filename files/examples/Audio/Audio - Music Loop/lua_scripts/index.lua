-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Music Loop"
    },
    
    onPreload = function(self)
        self.load:image("soundButton", "soundButton.png")

        -- Load audio. wav, ogg, mp3
        self.load:audio("myMusic", "We Are Home.ogg")
    end,

    onCreate = function(self)
        -- Add the audio asset to the AudioMaster.
        self.audio:createChild("Audio", {
            audio = "myMusic",
            loop = true -- Loop forever
        })

        self:createChild("Sprite", {
            texture = "soundButton",
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

                    -- Toggle playing music.
                    if not self.audio:isPlaying("myMusic") then
                        self.audio:play("myMusic")
                    else
                        self.audio:stop("myMusic")
                    end
                end
            }
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})