Nodes:load("Player")

Nodes:define("GameScene", "Scene", {
    onPreload = function(self)
        self.load:image("player", "player.png")
        self.load:spritesheet("tiles", "tiles.png", 16, 16)

        self.load:tilemap("map", "map.tmx")
    end,

    onCreate = function(self)
        self.get.player = self:createChild("Player")

        self.get.tilemap = self:createChild("Tilemap", {
            texture = "tiles",
            tilemap = "map"
        })
        self.get.player.collider:addCollisionTarget(self.get.tilemap)
        
        self.camera.followTarget = self.get.player
    end
})

Creator:createWorld({
    window = {
        width = 640,
        height = 360,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Top-down Player"
    },

    onCreate = function(self)
        -- Create Controls
        local left = Controls:scheme("left")
        left:setKeys( Key.Left, Key.A )

        local right = Controls:scheme("right")
        right:setKeys( Key.Right, Key.D )

        local up = Controls:scheme("up")
        up:setKeys( Key.Up, Key.W )

        local down = Controls:scheme("down")
        down:setKeys( Key.Down, Key.S )

        self:createChild("GameScene")
    end
})