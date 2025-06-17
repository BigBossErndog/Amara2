return NodeFactory:create("NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,
    onCreate = function(self)
        self.input:activate()
        self.input.draggable = true
        
        self.input:listen("onPointerDown", function(self, pointer)
            self:bringToFront()
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
    end,
    props = {
        isOpen = true
    },
    openBox = function(self, _height, _onEnd)
        self.visible = true
        self.props.isOpen = true
        self.tween:to({
            height = _height,
            duration = 0.1,
            onComplete = _onEnd
        })
    end,
    closeBox = function(self, _onEnd)
        if _onEnd == nil then
            _onEnd = function(self) 
                self.visible = false
            end
        end
        self.props.isOpen = false
        self.tween:to({
            height = 0,
            duration = 0.1,
            onComplete = _onEnd
        })
    end
})