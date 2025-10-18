-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,

        graphics = Graphics.OpenGL,
        
        title = "Follow The Mouse"
    },
    
    onPreload = function(self)
        -- The texture points right by default. Angle 0 points right (positive X direction).
        self.load:image("arrow", "arrow.png")
    end,

    onCreate = function(self)
        self.get.arrow = self:createChild("Sprite", {
            texture = "arrow"
        })
    end,

    onUpdate = function(self, deltaTime)
        self.get.arrow.rotation = math.angleBetween(self.get.arrow.pos, self.input.pointer)
        
        if math.distanceBetween(self.get.arrow.pos, self.input.pointer) > 4 then
            self.get.arrow.x = self.get.arrow.pos.x + math.cos(self.get.arrow.rotation) * 100 * deltaTime
            self.get.arrow.y = self.get.arrow.pos.y + math.sin(self.get.arrow.rotation) * 100 * deltaTime
        end
    end
})