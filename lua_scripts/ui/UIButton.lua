return NodeFactory:create("UIButton", "NineSlice", {
    texture = "uiButton",
    width = 16, height = 18,
    origin = 0,
    input = true,
    onCreate = function(self)
        self.props.enabled = true

        self.input:listen("onPointerDown", function(self, pointer)
            self.frame = 1
        end)
        self.input:listen("onPointerUp", function(self, pointer)
            self.frame = 0
            if self.props.enabled and self.func.onPress then
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

        if config.text then
            if not self.props.txt then
                self.props.txt = self:createChild("Text", {
                    font = "defaultFont",
                    origin = 0
                })
            end

            if Localize:has(config.text) then
                self.props.txt.text = Localize:get(config.text)
            else
                self.props.txt.text = config.text
            end

            self.width = self.props.txt.width + 16

            self.props.txt.pos = {
                math.floor(self.width/2.0 - self.props.txt.width/2.0),
                math.floor(self.height/2.0 - self.props.txt.height/2.0) - 2
            }
        end

        if config.toolTip then
            self.props.toolTip = config.toolTip
        end
    end,
    onUpdate = function(self, deltaTime)
        if self.frame == 0 then
            if self.props.icon then
                self.props.icon.y = self.height / 2.0
            end
            if self.props.txt then
                self.props.txt.y = math.floor(self.height/2.0 - self.props.txt.height/2.0) - 2
            end
        else
            if self.props.icon then
                self.props.icon.y = self.height / 2.0 + 1 
            end
            if self.props.txt then
                self.props.txt.y = math.floor(self.height/2.0 - self.props.txt.height/2.0) - 2 + 1 
            end
        end

        if self.props.toolTip and self.input.hovered then
            self.world.props.toolTips.func:showToolTip(self.props.toolTip, deltaTime)
        end
    end
})