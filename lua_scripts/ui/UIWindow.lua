return NodeFactory:create("UIWindow", "NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,
    onCreate = function(self)
        self.props.defWidth = self.width
        self.props.defHeight = self.height

        self.props.targetWidth = self.width
        self.props.targetHeight = self.height

        self.input:activate()
        self.input.draggable = true
        
        self.input:listen("onPointerDown", function(self, pointer)
            self:bringToFront()
        end)
        
        self.props.content = self:createChild("Group", {
            x = self.left,
            y = self.top,
            visible = false,
            alpha = 0
        })

        self.props.speed = 0.1

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.F4 },
            onPress = function()
                self.func:closeWindow(function()
                    self.world:destroy()
                end)
            end
        })

        self.func:closeInstantly()
    end,
    onUpdate = function(self, deltaTime)
        self.props.content:goTo(
            -self.props.targetWidth / 2.0,
            -self.props.targetHeight / 2.0
        )

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
        self.width = 0
        self.height = 0
        self.visible = false

        return self
    end,
    setTarget = function(self, _width, _height)
        if _width then
            self.props.targetWidth = _width
        end
        if _height then
            self.props.targetHeight = _height
        end
        return self
    end,
    openWindow = function(self, _onEnd)
        self.visible = true
        self.props.isOpen = true
        
        if not self.props.content.visible then
            self.props.content.visible = true
            self.props.content.alpha = 0
        end

        self.props.content.tween:to({
            alpha = 1,
            ease = Ease.SineIn,
            duration = self.props.speed
        })
        self.tween:to({
            width = self.props.targetWidth,
            height = self.props.targetHeight,
            duration = self.props.speed,
            ease = Ease.SineOut,
            onComplete = _onEnd
        })
    end,
    closeWindow = function(self, _onEnd)
        if _onEnd == nil then
            _onEnd = function(self) 
                self.visible = false
            end
        end

        self.props.content.tween:to({
            alpha = 0,
            ease = Ease.SineOut,
            duration = self.props.speed
        })

        self.props.isOpen = false
        self.tween:to({
            width = 0,
            height = 0,
            duration = self.props.speed,
            ease = Ease.SineIn,
            onComplete = _onEnd
        })
    end
})