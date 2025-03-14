-- game.scenes.add("testScene", "scenes/testScene");
-- game.factory.add("testEntity", "entities/testEntity");

game:configure({
    width = 480,
    height = 360,
    entry = "main"
});
game:run("scripts/testScript.lua");

