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
        -- You can "pointTowards" any Node, Pointer or Vector2 coordinate.
        --  self.get.arrow:pointTowards({ 8, 8 }) will point towards { 8, 8 }
        self.get.arrow:pointTowards(self.input.pointer)
        
        if math.distanceBetween(self.get.arrow.pos, self.input.pointer) > 4 then
            -- You can "moveTowards" any Node, Pointer or Vector2 coordinate.
            -- e.g. self.get.arrow:moveTowards({ 100, 100 }, 100 * deltaTime) to move towards { 100, 100 }
            self.get.arrow:moveTowards(self.input.pointer, 100 * deltaTime)
        end
    end
})