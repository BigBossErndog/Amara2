return Nodes:create("TerminalWindow", "UIWindow", {
    texture = "terminalWindow",
    width = 256,
    height = 120,

    onConfigure = function(self, config)
        if config.gameProcess then
            self.props.gameProcess = config.gameProcess
        end
    end,

    onCreate = function(self, config)
        local terminalWindowData = self.world.func:getSettings().terminalWindowData

        if terminalWindowData then
            self:goTo(
                terminalWindowData.x,
                terminalWindowData.y
            )
            self.props.targetWidth = terminalWindowData.width
            self.props.targetHeight = terminalWindowData.height
        end

        self.classes.UIWindow.func:onCreate(self, config)

        self.props.paddingLeft = 8
        self.props.paddingRight = 8
        self.props.paddingTop = 22
        self.props.paddingBottom = 8

        self.props.marginLeft = 2
        self.props.marginRight = 2
        self.props.marginTop = 2
        self.props.marginBottom = 2

        if not self.props.poolSize then
            self.props.poolSize = 24
        end

        if self.props.gameProcess then
            self.props.messages = self.props.gameProcess.output
        elseif not self.props.messages then
            self.props.messages = {}
        end

        self.props.lineSpacing = 2

        self.props.content:createChild("Text", {
            font = "defaultFont",
            text = Localize:get("title_printLog"),
            color = "#b1d7e9",
            origin = { 0, 0 },
            x = 10, y = 6
        })

        self.props.cont = self.props.content:createChild("Container", {
            x = self.props.paddingLeft,
            y = self.props.paddingTop,
            width = self.props.targetWidth - self.props.paddingLeft - self.props.paddingRight,
            height = self.props.targetHeight - self.props.paddingTop - self.props.paddingBottom,
            origin = 0
        })

        self.props.pool = self.props.cont:createChild("NodePool", {
            x = self.props.cont.left + self.props.marginLeft,
            y = self.props.cont.top + self.props.marginTop,
        })
        self.props.activePool = {}
        
        self.props.wallHeight = 0;

        for i = 1, self.props.poolSize do
            local item = self.props.pool:createChild("Text", {
                font = "defaultFont",
                color = Colors.White,
                origin = 0,
                wrapMode = WrapMode.ByWord
            })
            if i <= #self.props.messages then
                item:activate()
                item.text = self.props.messages[i]

                item.y = self.props.wallHeight
                self.props.wallHeight = self.props.wallHeight + item.height + self.props.lineSpacing

                table.insert(self.props.activePool, item)
            end
        end

        self.props.exitButton = self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            y = 4,
            icon = 0,
            onUpdate = function(button, deltaTime)
                button.classes.UIButton.func:onUpdate(deltaTime)

                button.x = self.props.targetWidth - button.width - 4
            end,
            onPress = function()
                self.func:closeWindow(function(self)
                    self:destroy()
                end)
                self.props.enabled = false

                if self.func.onExit then
                    self.func:onExit()
                end
            end
        })

        self.input:listen("onPointerUp", function(self)
            self.func:savePosition()
        end)
    end,

    pipeMessage = function(self, msg)
        table.insert(self.props.messages, msg)

        local item = self.props.pool:grab()

        if not item then
            item = table.remove(self.props.activePool, 1)
        end

        item:activate()
        item.text = msg
        
        if string.find(msg, "%f[%w]Error%f[%W]") then
            item.color = Colors.Red
        else
            item.color = Colors.White
        end

        item.y = self.props.wallHeight
        self.props.wallHeight = self.props.wallHeight + item.height + self.props.lineSpacing

        table.insert(self.props.activePool, item)
    end,

    savePosition = function(self)
        local setting = self.world.func:getSettings()
        if not setting.terminalWindowData then
            setting.terminalWindowData = {}
        end

        setting.terminalWindowData.x = self.x
        setting.terminalWindowData.y = self.y

        setting.terminalWindowData.width = self.props.targetWidth
        setting.terminalWindowData.height = self.props.targetHeight

        self.world.func:saveSettings()
    end,

    onUpdate = function(self)
        self.classes.UIWindow.func:onUpdate(self)

        self.props.cont.width = self.props.targetWidth - self.props.paddingLeft - self.props.paddingRight
        self.props.cont.height = self.props.targetHeight - self.props.paddingTop - self.props.paddingBottom

        self.props.pool.x = self.props.cont.left + self.props.marginLeft

        local wrapWidth = self.props.cont.width - self.props.marginLeft - self.props.marginRight

        local lastitem = nil
        for i = 1, #self.props.activePool do
            local item = self.props.activePool[i]

            if item.wrapWidth ~= wrapWidth then
                item.wrapWidth = wrapWidth
            end

            if lastitem then
                item.y = lastitem.y + lastitem.height + self.props.lineSpacing
            end

            if i == 1 then
                if self.props.pool.y + item.y < self.props.cont.top + self.props.marginTop then
                    self.props.pool.y = self.props.cont.top + self.props.marginTop - item.y
                end
            end
            if i == #self.props.activePool then
                if self.props.pool.y + item.y + item.height > self.props.cont.bottom - self.props.marginBottom then
                    self.props.pool.y = self.props.cont.bottom - self.props.marginBottom - item.y - item.height
                end
            end

            lastitem = item
        end
    end,

    unbindGameProcess = function(self)
        self.props.gameProcess = nil
    end
})