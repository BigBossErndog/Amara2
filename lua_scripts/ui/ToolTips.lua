Nodes:define("ToolTips", "NineSlice", {
    id = "toolTip",
    texture = "toolTipBox",

    maxWidth = 320,
    maxHeight = 64,

    visible = false,
    alpha = 0,

    onCreate = function(self)
        local props = self.props

        props.txt = self:createChild("Text", {
            font = "defaultFont",
            color = Colors.Black,
            text = "Test",
            origin = 0
        })

        props.justShown = false
        props.attempt = 0
    end,

    showToolTip = function(self, str, deltaTime)
        if str ~= self.get.check then
            self.get.check = str
            self.get.attempt = 0
            return
        end

        self.get.attempt = self.get.attempt + deltaTime
        if self.get.attempt < 0.6 then
            return
        end

        if Localize:has(str) then
            self.get.txt.text = Localize:get(str)
        else
            self.get.txt.text = str
        end
        
        self.width = self.get.txt.width + 8
        self.height = self.get.txt.height + 4

        self.get.txt:goTo(
            math.ceil(-self.get.txt.width/2.0),
            math.floor(-self.get.txt.height/2.0)
        )
        
        self.func:followMouse()

        self:bringToFront()
        
        self.visible = true
        self.get.justShown = true
    end,

    followMouse = function(self)
        self:goTo(
            self.input.mouse.x - self.right,
            self.input.mouse.y - self.height
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

    onUpdate = function(self, deltaTime)
        if self.visible then
            self.func:followMouse()
        end
        if self.get.justShown then
            self.alpha = self.alpha + deltaTime/0.16
            if self.alpha >= 1 then
                self.alpha = 1
            end
            self.get.justShown = false
        elseif self.visible and self.alpha > 0 then
            self.get.attempt = 0
            self.get.check = nil
            self.alpha = self.alpha - deltaTime/0.16
            if self.alpha <= 0 then
                self.alpha = 0
                self.visible = false
            end
        end
    end
})