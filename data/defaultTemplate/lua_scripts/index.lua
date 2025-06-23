-- Welcome to your new world

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,

        screenMode = ScreenMode.Windowed,
        
        title = "${Window_Title}"
    },

    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
    end,

    onCreate = function(self)
        self:createChild("Text", {
            font = "defaultFont",
            text = "Hello World!"
        })
    end,

    onUpdate = function(self)

    end
})