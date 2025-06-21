return NodeFactory:create("UIWindow", "NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,
    onCreate = function(self)
        self.props.defHeight = self.height
        self.props.targetHeight = self.height

        self.input:activate()
        self.input.draggable = true
        
        self.input:listen("onPointerDown", function(self, pointer)
            self:bringToFront()
        end)

        self.props.content = self:createChild("Group", {
            x = self.left,
            y = self.top,
            visible = false
        })

        self.props.speed = 0.1

        self.func:closeInstantly()
    end,
    onUpdate = function(self, deltaTime)
        self.props.content:goTo(self.left, -self.props.targetHeight / 2.0)

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
    showContent = function(self)
        self.props.content.visible = true
    end,
    hideContent = function(self)
        self.props.content.visible = false
    end,
    closeInstantly = function(self)
        self.height = 0
        self.visible = false
    end,
    openBox = function(self, _height, _onEnd)
        self.visible = true
        self.props.isOpen = true

        if _height == nil then
            _height = self.props.defHeight
        end
        self.props.targetHeight = _height
        
        if not self.props.content.visible then
            self.props.content.visible = true
            self.props.content.alpha = 0
        end

        self.tween:to({
            height = _height,
            duration = self.props.speed,
            onProgress = function(self, progress)
                self.props.content.alpha = progress
            end,
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
            duration = self.props.speed,
            onProgress = function(self, progress)
                if self.props.content and self.props.content.visible then
                    self.props.content.alpha = 1 - progress
                end
            end,
            onComplete = _onEnd
        })
    end,
    closeBoxAndDestroy = function(self, _onEnd)
        if _onEnd == nil then
            _onEnd = function(self) 
                self.props.content:destroyChildren()
            end
        end
        self.closeBox(_onEnd)
        self.props.content.visible = true
    end
})