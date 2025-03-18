-- game.scenes.add("testScene", "scenes/testScene");
-- game.factory.add("testEntity", "entities/testEntity");

-- game:configure_override = function(config)
--     if config["hello"] then log("HELLO!") end
-- end

-- scripts:run("scripts/testScript.lua");

game:uncapFPS()

local w = creator:createWorld()
local e = w:createChild("Entity")

e.onUpdate = function(self, delta)
    log("FPS: ", math.floor(game.fps))
end
