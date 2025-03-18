game:uncapFPS()

local w = creator:createWorld()

local e = w:createChild("Entity")
e.id = "acting entity"

local a = e:createChild("Action")
local t = e.tween:to({
    x = 10,
    duration = 1
}):to({
    x = 0,
    duration = 1
})
e.onUpdate = function(self, delta)
    log(math.floor(e.x))
end