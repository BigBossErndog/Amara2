-- game:uncapFPS()

-- local w = creator:createWorld()

-- game:setTargetFPS(60)

-- local e = w:createChild("Entity")
-- e.id = "acting entity"

-- w.onUpdate = function(self) log(math.floor(game.fps)) end
-- w.id = "hi"
-- e.onUpdate = function(self) log(world.x) end

-- print_metatable(w)

-- local t = e.tween:to({
--     x = 10,
--     duration = 1,
--     onUpdate = function(self)
--         log(e.x)
--     end,
--     ease = Ease.SineOut
-- }):to({
--     x = 0,
--     duration = 1,
--     onUpdate = function(self)
--         log(e.x)
--     end,
--     onComplete = function(self)
--         -- self.world:destroy()
--     end
-- })

scripts:run("scripts/testScript")