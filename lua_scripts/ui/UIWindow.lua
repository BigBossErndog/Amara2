return NodeFactory:create("NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,
    onCreate = function(self)
        self.input.draggable = true
        print(self)
        print(self.width, self.height)
        print(self.maxWidth, self.maxHeight)

        self.input:listen("onPointerDown", function(self, pointer)
            self:bringToFront()
        end)

        self.input:listen("onPointerUp", function(self, pointer)
            if pointer.state.timeHeld < 0.15 then
                self.tween:from({
                    rotation = 0
                }):to({
                    rotation = math.pi,
                    duration = 1,
                    ease = Ease.SineInout
                })
            end
        end)
    end,
    onUpdate = function(self)
        if self.x < self.world.left + self.width/2 then
            self.x = self.world.left + self.width/2
        elseif self.x > self.world.right - self.width/2 then
            self.x = self.world.right - self.width/2
        end
        if self.y < self.world.top + self.height/2 then
            self.y = self.world.top + self.height/2
        elseif self.y > self.world.bottom - self.height/2 then
            self.y = self.world.bottom - self.height/2
        end
    end
})