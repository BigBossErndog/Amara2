Nodes:load("Player")

Nodes:define("GameScene", "Scene", {
    onCreate = function(self)
        -- Reconfigure the main camera

        self.get.tilemap = self:createChild("Tilemap", {
            texture = "tiles",
            tilemap = "map"
        })

        self.camera:configure({
            origin = { 0, 0 },
            -- Set size to fill left half of screen
            size = { self.world.view.left, self.world.view.top, self.world.view.width/2, self.world.view.height },
            lerp = 0.95
        })


        -- Create a secondary camera.
        self.get.camera2 = self:createChild("Camera", {
            origin = { 0, 0 },
            size = { self.world.view.center.x, self.world.view.top, self.world.view.width/2, self.world.view.height },
            lerp = 0.95
        })

        -- Make Player 1
        self.get.player1 = self:createChild("Player", {
            x = -32,
            tint = "green",
            leftButton = Key.A,
            rightButton = Key.D,
            upButton = Key.W,
            downButton = Key.S
        })
        -- First camera follows Player 1
        self.camera.followTarget = self.get.player1
        -- Set collision with tilemap
        self.get.player1.collider:addCollisionTarget(self.get.tilemap)

        -- Make Player 2
        self.get.player2 = self:createChild("Player", {
            x = 32,
            tint = "red",
            leftButton = Key.Left,
            rightButton = Key.Right,
            upButton = Key.Up,
            downButton = Key.Down
        })
        -- Second camera follows Player 2
        self.get.camera2.followTarget = self.get.player2
        -- Set collision with tilemap
        self.get.player2.collider:addCollisionTarget(self.get.tilemap)
    end
})

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Split-Screen"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        self.load:image("player", "player.png")

        self.load:tilemap("map", "map.tmx")
        self.load:spritesheet("tiles", "tiles.png", 32, 32)
    end,

    onCreate = function(self)
        self:createChild("GameScene")

        self:createChild("Text", {
            origin = { 0, 1 },
            x = self.world.view.left + 4,
            y = self.world.view.bottom - 4,
            color = "pink",
            font = "defaultFont",
            text = "WASD"
        })

        self:createChild("Text", {
            origin = { 1, 1 },
            x = self.world.view.right - 4,
            y = self.world.view.bottom - 4,
            color = "pink",
            font = "defaultFont",
            text = "Arrow Keys"
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})