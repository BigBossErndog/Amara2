-- game.scenes.add("testScene", "scenes/testScene");
-- game.factory.add("testEntity", "entities/testEntity");

-- game:configure_override = function(config)
--     if config["hello"] then log("HELLO!") end
-- end
game.scripts:run("scripts/testScript.lua");

return ""