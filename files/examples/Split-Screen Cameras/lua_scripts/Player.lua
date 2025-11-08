Nodes:define("Player", "Sprite", {
    texture = "player",

    collider = true, -- Assign true to create a collider. You can configure its properties if you assign a table instead.

    onConfigure = function(self, config)
        if config.leftButton then
            self.get.leftButton = config.leftButton
        end
        if config.rightButton then
            self.get.rightButton = config.rightButton
        end
        if config.upButton then
            self.get.upButton = config.upButton
        end
        if config.downButton then
            self.get.downButton = config.downButton
        end
    end,

    onUpdate = function(self)
        self.collider.velocity.x = 0
        self.collider.velocity.y = 0

        if Keyboard:isDown(self.get.leftButton) then
            self.collider.velocity.x = self.collider.velocity.x - 200
        end
        if Keyboard:isDown(self.get.rightButton) then
            self.collider.velocity.x = self.collider.velocity.x + 200
        end
        if Keyboard:isDown(self.get.upButton) then
            self.collider.velocity.y = self.collider.velocity.y - 200
        end
        if Keyboard:isDown(self.get.downButton) then
            self.collider.velocity.y = self.collider.velocity.y + 200
        end
    end
})