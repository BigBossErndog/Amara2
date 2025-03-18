game:uncapFPS()

local w = creator:createWorld()

game:setTargetFPS(60)

local e = w:createChild("Entity")
e.id = "acting entity"

-- e.onUpdate = function(self) log("ENTITY LIVES") end

local a = e:createChild("Action")
local t = e.tween:to({
    x = 10,
    duration = 1,
    onUpdate = function(self)
        log(e.x)
    end
}):to({
    x = 0,
    duration = 1,
    onUpdate = function(self)
        log(e.x)
    end,
    onComplete = function(self)
        self.world:destroy()
    end
})