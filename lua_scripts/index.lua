local rect;

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        transparent = true,
        clickThrough = true,
        alwaysOnTop = true,
        graphics = Graphics.OpenGL
    },
    onPreload = function(world) 
        world.load:image("uiBox", "ui/amara2_uiBox.png");
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16);
    end
})