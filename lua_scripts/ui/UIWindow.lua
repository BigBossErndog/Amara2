Nodes:define("UIWindow", "NineSlice", {
    texture = "uiBox",
    maxWidth = 640, maxHeight = 640,
    width = 128, height = 64,
    input = true,

    props = {
        isOpen = false,
        speed = 0.1
    },
    
    onCreate = function(self)
        self.get.defWidth = self.width
        self.get.defHeight = self.height
        
        self.get.targetWidth = self.width
        self.get.targetHeight = self.height

        self.input:activate()
        self.input.draggable = true
        
        self.input:listen("onPointerDown", function(self, pointer)
            self:bringToFront()
            self.world.get.focusedWindow = self
        end)

        self.get.contentRoot = self:createChild("Group", {
            x = 0, y = 0,
            scale = 0
        })
        
        self.get.content = self.get.contentRoot:createChild("Group", {
            x = self.left,
            y = self.top
        })

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
        self.get.content:goTo(
            -self.get.targetWidth / 2.0,
            -self.get.targetHeight / 2.0
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
    showContent = function(self)
        self.get.content.visible = true
    end,
    hideContent = function(self)
        self.get.content.visible = false
    end,
    closeInstantly = function(self)
        self.width = 0
        self.height = 0
        self.visible = false

        return self
    end,
    showInstantly = function(self)
        self.width = self.get.targetWidth
        self.height = self.get.targetHeight
        self.visible = true
        self.get.content.visible = true
        self.get.contentRoot.scale = 1

        return self
    end,
    setTarget = function(self, _width, _height)
        if _width then
            self.get.targetWidth = _width
        end
        if _height then
            self.get.targetHeight = _height
        end
        return self
    end,
    openWindow = function(self, _onEnd)
        if self.get.isOpen then
            return
        end
        self.visible = true
        self.get.isOpen = true
        
        if not self.get.content.visible then
            self.get.content.visible = true
            self.get.contentRoot.scale = 0
        end

        self.world.get.focusedWindow = self

        self.get.contentRoot.tween:to({
            scaleX = 1,
            scaleY = 1,
            ease = Ease.SineOut,
            duration = self.get.speed
        })
        self.tween:to({
            width = self.get.targetWidth,
            height = self.get.targetHeight,
            duration = self.get.speed,
            ease = Ease.SineOut,
            onComplete = function()
                if _onEnd then
                    _onEnd(self)
                end
                if self.func.onWindowOpen then
                    self.func:onWindowOpen()
                end
            end
        })
    end,
    closeWindow = function(self, _onEnd)
        if not self.get.isOpen then
            return
        end
        if _onEnd == nil then
            _onEnd = function(self) 
                self.visible = false
            end
        end

        self.get.contentRoot.tween:to({
            scaleX = 0,
            scaleY = 0,
            ease = Ease.SineIn,
            duration = self.get.speed
        })

        self.get.isOpen = false

        self.tween:to({
            width = 0,
            height = 0,
            duration = self.get.speed,
            ease = Ease.SineIn,
            onComplete = function()
                if _onEnd then
                    _onEnd(self)
                end
                if self.func.onWindowClose then
                    self.func:onWindowClose()
                end
            end
        })
    end,

    onDestroy = function(self)
        if self.world.get.focusedWindow == self then
            self.world.get.focusedWindow = nil
        end
    end
})