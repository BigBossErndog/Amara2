Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "NineSlice"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        self.load:image("uiBox", "uiBox.png")
        -- Note: You can also use a spritesheet and use different frames.
    end,

    onCreate = function(self)
        -- A NineSlice is useful for a variable size box without stretching the edges.

        self.get.box = self:createChild("NineSlice", {
            texture = "uiBox",

            width = 128,
            height = 128,
            
            -- maxWidth and maxHeight is not required (auto-sets to width and height), but if you plan to change the size a lot it will be useful for preventing lag.
            -- Setting a width/height greater than maxWidth/maxHeight can cause a lag spike as it recreates its texture.
            maxWidth = 256,
            maxHeight = 256,

            -- Margins define how the box is split up.
            -- If omitted, it will split up the box into thirds.
            -- The middle sections are stretched to make overall box match width and height.
            marginLeft = 4,
            marginRight = 4,
            marginTop = 4,
            marginBottom = 4

            -- If they are all the same you can also use:
            -- margins = 4
        })

        self.get.myText = self.get.box:createChild("Text", {
            font = "defaultFont",
            text = "This is a box.",
            origin = { 0, 0 },
            onUpdate = function(txt)
                -- Keep this text to the top-left of the box
                txt.x = self.get.box.left + 3
                txt.y = self.get.box.top + 2

            end
        })

        -- You can change the size of the NineSlice with tweens.
        self.get.box.tween:to({
            width = self.get.myText.width + 6,
            height = self.get.myText.height + 8,
            duration = 1,
            interim = 0.5,
            ease = Ease.SineInOut,
            repeats = -1,
            yoyo = true
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})