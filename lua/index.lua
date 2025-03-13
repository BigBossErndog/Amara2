game.scenes.add("testScene", "scenes/testScene");
game.factory.add("testEntity", "entities/testEntity");

game.run("testScript");

game.configure({
    width: 480,
    height: 360,
    entry: "main"
});