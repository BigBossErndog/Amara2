local platform = self:createChild("FillRect", {
    width = 256,
    height = 32
})

local playerSprite = self:createChild("FillRect", {
    width = 32,
    height = 64,
    
    y = -100
    
    collider = {
        target = platform,
        acceleration = { 0, 1000 } -- add gravity, in pixels/second so numbers are usually big
        -- targets = { platform1, platform2, platform3 } for multiple targets
    },

    onUpdate = function(self)
        if self.collider.collisionDirection & Direction.Down ~= 0 then
                
        end
    end,
    
    jump = function(self)
        self.collider.velocity.y = -500
    end
})