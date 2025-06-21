return NodeFactory:create("UIButton", "NineSlice", {
    texture = "uiButton",
    width = 16, height = 18,
    origin = 0,
    input = true,
    onCreate = function(self)
        self.input:listen("onPointerDown", function(self, pointer)
            self.frame = 1
        end)
        self.input:listen("onPointerUp", function(self, pointer)
            self.frame = 0
            if self.func.onPress then
                self.func:onPress()
            end
        end)
        self.input:listen("onPointerExit", function(self, pointer)
            self.frame = 0
        end)
    end,
    configure = function(self, config)
        self:super_configure(config)

        if config.icon then
            if not self.props.icon then
                self.props.icon = self:createChild("Sprite", {
                    texture = "uiIcons",
                    x = self.width / 2.0,
                    y = self.height / 2.0,
                    origin = 0.5,
                    visible = false
                })
            end
            self.props.icon.frame = config.icon
            self.props.icon.visible = true
        end

        if config.toolTip then
            self.props.toolTip = config.toolTip
        end
    end,
    onUpdate = function(self, deltaTime)
        if self.frame == 0 then
            self.props.icon.y = self.height / 2.0
        else
            self.props.icon.y = self.height / 2.0 + 1 
        end

        if self.props.toolTip and self.input.hovered then
            self.world.props.toolTips.func:showToolTip(self.props.toolTip, deltaTime)
        end
    end
})