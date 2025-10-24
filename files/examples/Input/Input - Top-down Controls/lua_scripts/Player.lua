Nodes:define("Player", "Sprite", {
    texture = "player",

    collider = true, -- Assign true to create a collider. You can configure its properties if you assign a table instead.

    onUpdate = function(self)
        self.collider.velocity.x = 0
        self.collider.velocity.y = 0

        if Controls:isDown("left") then
            self.collider.velocity.x = self.collider.velocity.x - 200
        end
        if Controls:isDown("right") then
            self.collider.velocity.x = self.collider.velocity.x + 200
        end
        if Controls:isDown("up") then
            self.collider.velocity.y = self.collider.velocity.y - 200
        end
        if Controls:isDown("down") then
            self.collider.velocity.y = self.collider.velocity.y + 200
        end
    end
})