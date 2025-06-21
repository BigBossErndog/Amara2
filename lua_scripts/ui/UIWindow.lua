return NodeFactory:create("UIWindow", "NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,
    onCreate = function(self)
        self.props.defHeight = self.height

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
    end,
    onUpdate = function(self, deltaTime)
        self.props.content:goTo(self.left, self.top)

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
        
        if not self.props.content.visible then
            self.props.content.visible = true
            self.props.content.alpha = 1
            -- self.props.content.tween:to({
            --     alpha = 1,
            --     duration = 0.12
            -- })
        end

        self.tween:to({
            height = _height,
            duration = 0.12,
            onComplete = _onEnd
        })
    end,
    closeBox = function(self, _onEnd)
        self.props.content.visible = false
        if _onEnd == nil then
            _onEnd = function(self) 
                self.visible = false
            end
        end

        self.props.isOpen = false
        self.tween:to({
            height = 0,
            duration = 0.12,
            onComplete = _onEnd
        })
    end,
    closeBoxAndDestroy = function(self, _onEnd)
        self.closeBox(_onEnd)
        self.props.content.visible = true
        self.props.content.tween.to({
            alpha = 0,
            duration = 0.12,
            onComplete = function(self)
                self.visible = false
                self:destroyChildren()
            end
        })
    end
})