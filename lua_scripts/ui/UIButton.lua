Nodes:define("UIButton", "NineSlice", {
    texture = "uiButton",
    width = 16, height = 18,
    origin = 0,

    input = {
        active = true,
        cursor = Cursor.Pointer,
        onPointerDown = function(self, pointer)
            self.get.clicked = true
            if self.get.enabled then
                self.frame = 2
            end
        end,
        onPointerUp = function(self, pointer)
            self.frame = 1
            if self.get.enabled and self.func.onPress then
                self.func:onPress()
            end
        end,
        onPointerExit = function(self, pointer)
            self.frame = 1
        end
    },

    onCreate = function(self)
        self.get.enabled = true

        if self.get.hotkey then
            self:createChild("Hotkey", {
                config = self.get.hotkey,
                onPress = function()
                    self.func:forcePress()
                end
            })
        end
    end,
    
    onConfigure = function(self, config)
        if config.icon then
            if not self.get.icon then
                self.get.icon = self:createChild("Sprite", {
                    texture = "uiIcons",
                    x = self.width / 2.0,
                    y = self.height / 2.0,
                    origin = 0.5,
                    visible = false
                })
            end
            self.get.icon.frame = config.icon
            self.get.icon.visible = true
        end

        if config.text then
            if not self.get.txt then
                self.get.txt = self:createChild("Text", {
                    font = "defaultFont",
                    origin = 0
                })
            end

            if Localize:has(config.text) then
                self.get.txt.text = Localize:get(config.text)
            else
                self.get.txt.text = config.text
            end

            self.width = self.get.txt.width + 16

            self.get.txt.pos = {
                math.floor(self.width/2.0 - self.get.txt.width/2.0),
                math.floor(self.height/2.0 - self.get.txt.height/2.0) - 2
            }
        end

        if config.toolTip then
            self.get.toolTip = config.toolTip
        end

        if config.hotkey then
            self.get.hotkey = config.hotkey
        end
    end,

    onUpdate = function(self, deltaTime)
        if self.frame == 1 then
            if self.get.icon then
                self.get.icon.y = self.height / 2.0
            end
            if self.get.txt then
                self.get.txt.y = math.floor(self.height/2.0 - self.get.txt.height/2.0) - 2
            end
        else
            if self.get.icon then
                self.get.icon.y = self.height / 2.0 + 1
            end
            if self.get.txt then
                self.get.txt.y = math.floor(self.height/2.0 - self.get.txt.height/2.0) - 2 + 1 
            end
        end
        if self.get.toolTip and self.get.enabled and self.input.hovered then
            if not self.get.clicked then
                self.world.get.toolTips.func:showToolTip(self.get.toolTip, deltaTime)
            end
        else
            self.get.clicked = nil
        end
    end,

    setIcon = function(self, frame)
        if self.get.icon then
            self.get.icon.frame = frame
        end
    end,

    forcePress = function(self)
        self.frame = 2
        if self.get.enabled and self.func.onPress then
            self.func:onPress()
        end
        self:wait(0.1):next(function()
            self.frame = 1
        end)
    end
})