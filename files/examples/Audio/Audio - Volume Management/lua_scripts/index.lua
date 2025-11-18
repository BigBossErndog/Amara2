Nodes:load("Slider") -- Loads the Slider.lua file.

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Music Volume Management"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        -- Load audio. wav, ogg, mp3
        self.load:audio("myMusic", "We Are Home.ogg")
        self.load:audio("selectSFX", "select.ogg")
    end,

    createMusic = function(self)
        local musicGroup = self.audio:createChild("AudioGroup", {
            id = "music",
            volume = 0.8
        })

        musicGroup:createChild("Audio", {
            audio = "myMusic",
            loop = true
        })

        -- Add more Audio to musicGroup to manage volume of all music.
    end,

    createSFX = function(self)
        local sfxGroup = self.audio:createChild("AudioGroup", {
            id = "sfx"
        })

        sfxGroup:createChild("Audio", {
            audio = "selectSFX"
        })

        -- You can add more to sfxGroup to manage volume of all SFX.
    end,

    onCreate = function(self)
        self.func:createMusic()
        self.func:createSFX()

        self.audio:play("music/myMusic") -- Must include groups.

        self:createChild("Slider", {
            text = "Master Volume",
            y = -32,
            onChange = function(val)
                -- Use masterVolume to manage overall volume, i.e. User setting
                -- You can use self.audio.volume = val to manage fading in/out, control volume to match ambience, etc.
                self.audio.masterVolume = val

                self.audio:play("sfx/selectSFX")
            end
        })

        self:createChild("Slider", {
            text = "Music Volume",
            y = 0,
            onChange = function(val)
                self.audio.get["music"].volume = val

                self.audio:play("sfx/selectSFX")
            end
        })

        self:createChild("Slider", {
            text = "SFX Volume",
            y = 32,
            onChange = function(val)
                self.audio.get["sfx"].volume = val

                self.audio:play("sfx/selectSFX")
            end
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})