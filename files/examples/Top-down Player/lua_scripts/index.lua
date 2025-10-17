-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Top-down Player"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
    end,

    onCreate = function(self)
        self.get.myText = self:createChild("Text", {
            font = "defaultFont",
            text = "Hello World!"
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})