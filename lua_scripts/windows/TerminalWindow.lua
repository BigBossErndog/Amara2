return Nodes:define("TerminalWindow", "UIWindow", {
    texture = "terminalWindow",
    width = 256,
    height = 100,

    input = true,

    onConfigure = function(self, config)
        if config.gameProcess then
            self.props.gameProcess = config.gameProcess
        end
        if config.disableSavePosition then
            self.props.disableSavePosition = config.disableSavePosition
        end
        if config.allowMinimize then
            self.props.allowMinimize = config.allowMinimize
        end
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self, config)
        local settings = self.world.func:getSettings()
        local terminalWindowData = settings.terminalWindowData
        
        if not self.props.disableSavePosition then
            if terminalWindowData then
                if terminalWindowData.x and terminalWindowData.y then
                    self:goTo(
                        terminalWindowData.x,
                        terminalWindowData.y
                    )
                end
                if terminalWindowData.width and terminalWindowData.height then
                    self.props.targetWidth = terminalWindowData.width
                    self.props.targetHeight = terminalWindowData.height
                end
            else
                settings.terminalWindowData = {
                    darkened = false
                }
                terminalWindowData = settings.terminalWindowData

                self.world.func:saveSettings()
            end
        elseif not terminalWindowData then
            terminalWindowData = {
                darkened = false
            }
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

        self.props.bottomLocked = true

        if not self.props.poolSize then
            self.props.poolSize = 32
        end

        if self.props.gameProcess then
            self.props.messages = self.props.gameProcess.output
        end
        if not self.props.messages then
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
        local wrapWidth = self.props.cont.width - self.props.marginLeft - self.props.marginRight

        for i = 1, self.props.poolSize do
            local item = self.props.pool:createChild("Text", {
                font = "defaultFont",
                color = Colors.White,
                origin = 0,
                wrapMode = WrapMode.ByWord
            })
        end

        if self.props.messages and #self.props.messages > 0 then
            for i = 1, #self.props.messages do
                local msg = self.props.messages[i]
                self.func:pipeMessage(msg)
            end
        end

        local buttonPosX = 0
        local buttonSpacing = 20

        self.props.exitButton = self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            y = 4,
            icon = 1,
            props = {
                buttonPosX = buttonPosX
            },
            onUpdate = function(button, deltaTime)
                button.classes.UIButton.func:onUpdate(deltaTime)

                button.x = self.props.targetWidth - button.width - 4 - button.props.buttonPosX
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

        if self.props.allowMinimize then
            buttonPosX = buttonPosX + buttonSpacing
            self.props.content:createChild("UIButton", {
                id = "minimizeButton",
                toolTip = "toolTip_minimize",
                y = 4,
                icon = 4,
                props = {
                    buttonPosX = buttonPosX
                },
                onUpdate = function(button, deltaTime)
                    button.classes.UIButton.func:onUpdate(deltaTime)

                    button.x = self.props.targetWidth - button.width - 4 - button.props.buttonPosX
                end,
                onPress = function(self)
                    self.world:minimizeWindow()
                end
            })
        end

        buttonPosX = buttonPosX + buttonSpacing

        self.frame = terminalWindowData.darkened and 2 or 1
        self.props.darkenButton = self.props.content:createChild("UIButton", {
            id = "darkenButton",
            toolTip = "toolTip_toggleBGOpacity",
            y = 4,
            icon = terminalWindowData.darkened and 15 or 14,
            props = {
                buttonPosX = buttonPosX
            },
            onUpdate = function(button, deltaTime)
                button.classes.UIButton.func:onUpdate(deltaTime)

                button.x = self.props.targetWidth - button.width - 4 - button.props.buttonPosX
            end,
            onPress = function()
                terminalWindowData.darkened = not terminalWindowData.darkened
                self.props.darkenButton.func:setIcon(terminalWindowData.darkened and 15 or 14)

                self.frame = terminalWindowData.darkened and 2 or 1

                self.world.func:saveSettings()
            end
        })

        
        self.props.scrollBar = self.props.content:createChild("FillRect", {
            color = { 80, 80, 80 },
            width = 2,
            origin = 0,
            visible = false,
            
            onCreate = function(scrollBar)
                scrollBar.props.pos = scrollBar:createChild("FillRect", {
                    color = { 200, 200, 200 },
                    width = 2,
                    height = 1,
                    origin = 0
                })

            end,
            manageScrollPosition = function(scrollBar)
                scrollBar.visible = true

                scrollBar.x = self.props.cont.x + self.props.cont.width + scrollBar.width - 1
                scrollBar.y = self.props.cont.y + 2
                
                scrollBar.height = self.props.cont.height - 4

                local pos = scrollBar.props.pos
                
                local firstItem = self.props.activePool[1]
                local lastItem = self.props.activePool[#self.props.activePool]

                pos.height = scrollBar.height * ((self.props.cont.height - self.props.marginBottom - self.props.marginTop) / (self.props.wallHeight + self.props.marginBottom + self.props.marginTop - firstItem.y))
                pos.y = -(scrollBar.height - pos.height) * ((-self.props.pool.y + self.props.cont.top + self.props.marginTop - firstItem.y) / ((self.props.cont.bottom - self.props.marginBottom - lastItem.y - lastItem.height) - (self.props.cont.top + self.props.marginTop - firstItem.y)))
            end
        })

        self.input:listen("onPointerUp", function(self)
            self.func:savePosition()
        end)
    end,

    checkForError  = function(self, msg, item)
        local ret = false
        
        if string.contains(msg, "[string ") then
            item.color = Colors.Red
            local filename, details = string.match(msg, '%[string "([^"]+)"]:(.*)')
            if filename and details then
                item.text = string.format('Error in "%s" at line %s', filename, details)
            end
            item.text = string.gsub(item.text, "%[string ", "[")
            ret = true
        elseif string.starts_with(msg, "caught (...) exception") then
            item.text = "Error: Invalid property or argument assignment."
            item.color = Colors.Red
            self.props.allowTrace = true
            ret = true
        elseif string.starts_with(msg, "Error: ") then
            item.color = Colors.Red
            self.props.allowTrace = true
            ret = true
        elseif string.starts_with(msg, "[json.exception.type_error.302]") then
            local details = string.match(msg, '%[json.exception.type_error.302] type(.*)')
            details = string.gsub(details, "null", "object")
            item.text = "Error: Type" .. details
            item.color = Colors.Red
            self.props.allowTrace = true
            ret = true
        else
            if msg == "Program aborted unexpectedly." then
                item.color = "#5d00ff"
            elseif string.contains(msg, "rendering to window using") then
                item.color = Colors.Cyan
            elseif string.starts_with(msg, "Note:") then
                item.color = Colors.Yellow
            else
                item.color = Colors.White
            end
            ret = false
        end
        item.props.defColor = item.color
        item.props.isError = ret

        local filePath = string.match(msg, '"([^"]+%.lua[c]?)"')
        if filePath and string.len(filePath) > 0 then
            if self.props.projectPath then
                local targetPath = System:join(self.props.projectPath, "lua_scripts", filePath)
                local settings = self.world.func:getSettings()
                if System:exists(targetPath) then
                    item.input:activate()
                    item.input.cursor = Cursor.Pointer
                    item.input:listen("onPointerUp", function(txt)
                        OpenCodeEditor(settings, self.props.projectPath, targetPath)

                        txt.color = txt.props.isError and Colors.White or "#82adc2"
                        txt.tween:to({
                            color = txt.props.defColor,
                            duration = 0.1,
                            onComplete = function(txt)
                                txt.color = txt.props.defColor
                            end
                        })
                    end)
                    item.input:listen("onPointerHover", function(txt)
                        if txt.props.isError then
                            txt.color = "#ff4646"
                        else
                            txt.color = "#c5ecff"
                        end
                        txt.props.defColor = txt.color
                    end)
                    item.input:listen("onPointerExit", function(txt)
                        if txt.props.isError then
                            txt.color = Colors.Red
                        else
                            txt.color = Colors.White
                        end
                        txt.props.defColor = txt.color
                    end)
                else
                    item.input:stopListening("onPointerUp")
                end
            end
        end

        return ret
    end,

    pipeMessage = function(self, msg)
        if string.starts_with(msg, "\t[C") or string.starts_with(msg, "stack traceback") then
            return;
        end
        if string.starts_with(msg, "\t[string") and not self.props.allowTrace then
            return;
        end
        self.props.allowTrace = false

        table.insert(self.props.messages, msg)

        local item = self.props.pool:grab()
        
        if not item then
            item = table.remove(self.props.activePool, 1)
        end

        item:activate()
        item.text = msg
        item.input:stopListening()
        item.input:deactivate()

        self.func:checkForError(msg, item)

        local wrapWidth = self.props.cont.width - self.props.marginLeft - self.props.marginRight
        if item.wrapWidth ~= wrapWidth then
            item.wrapWidth = wrapWidth
        end

        if self.props.wallHeight > 0 then
            self.props.wallHeight = self.props.wallHeight + self.props.lineSpacing
        end
        item.y = self.props.wallHeight
        self.props.wallHeight = self.props.wallHeight + item.height

        table.insert(self.props.activePool, item)

        self.props.pool.y = self.props.cont.bottom - self.props.marginBottom - item.y - item.height
        self.props.bottomLocked = true

        return item
    end,

    handleMessage = function(self, msg)
        return self.func:pipeMessage(msg)
    end,

    savePosition = function(self)
        if not self.props.disableSavePosition then
            local setting = self.world.func:getSettings()
            if not setting.terminalWindowData then
                setting.terminalWindowData = {}
            end

            setting.terminalWindowData.x = self.x
            setting.terminalWindowData.y = self.y

            setting.terminalWindowData.width = self.props.targetWidth
            setting.terminalWindowData.height = self.props.targetHeight

            self.world.func:saveSettings()
        end
    end,

    onSizeChange = function(self)
        local wrapWidth = self.props.cont.width - self.props.marginLeft - self.props.marginRight
        local lastitem = nil
        self.props.wallHeight = 0
        if #self.props.activePool > 0 then
            for i = 1, #self.props.activePool do
                local item = self.props.activePool[i]

                if item.wrapWidth ~= wrapWidth then
                    item.wrapWidth = wrapWidth
                end

                if lastitem then
                    item.y = lastitem.y + lastitem.height + self.props.lineSpacing
                else
                    item.y = 0
                end
                lastitem = item
            end
            self.props.wallHeight = lastitem.y + lastitem.height
        end
    end,

    onUpdate = function(self)
        self.classes.UIWindow.func:onUpdate(self)

        if self.input.hovered then
            self.props.pool.y = self.props.pool.y + self.input.mouse.wheel.y * 5

            if self.input.mouse.wheel.y ~= 0 and self.props.wallHeight > (self.props.cont.height - self.props.marginBottom - self.props.marginTop) then
                self.props.bottomLocked = false
            end
        end

        self.props.cont.width = self.props.targetWidth - self.props.paddingLeft - self.props.paddingRight
        self.props.cont.height = self.props.targetHeight - self.props.paddingTop - self.props.paddingBottom
        self.props.pool.x = self.props.cont.left + self.props.marginLeft

        if #self.props.activePool > 0 then
            local firstItem = self.props.activePool[1]
            local lastItem = self.props.activePool[#self.props.activePool]
            local viewableHeight = self.props.cont.height - self.props.marginBottom - self.props.marginTop

            if self.props.wallHeight <= viewableHeight then
                self.props.pool.y = self.props.cont.top + self.props.marginTop - firstItem.y
                self.props.bottomLocked = true
            else
                if self.props.bottomLocked then
                    self.props.pool.y = self.props.cont.bottom - self.props.marginBottom - lastItem.y - lastItem.height
                else
                    if self.props.pool.y + firstItem.y > self.props.cont.top + self.props.marginTop then
                        self.props.pool.y = self.props.cont.top + self.props.marginTop - firstItem.y
                    end
                    if self.props.pool.y + lastItem.y + lastItem.height < self.props.cont.bottom - self.props.marginBottom then
                        self.props.pool.y = self.props.cont.bottom - self.props.marginBottom - lastItem.y - lastItem.height
                        self.props.bottomLocked = true
                    end
                end
            end
        end
        
        if self.props.wallHeight > (self.props.cont.height - self.props.marginBottom - self.props.marginTop) then
            self.props.scrollBar.func:manageScrollPosition(self.props.scrollBar)
        else
            self.props.scrollBar.visible = false
        end
    end,

    unbindGameProcess = function(self)
        self.props.gameProcess = nil
    end
})