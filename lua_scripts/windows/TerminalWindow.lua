Nodes:define("TerminalWindow", "UIWindow", {
    texture = "terminalWindow",
    width = 256,
    height = 100,

    input = true,

    props = {
        titleText = "title_printLog",
        poolSize = 256
    },

    onConfigure = function(self, config)
        if config.gameProcess then
            self.get.gameProcess = config.gameProcess
        end
        if config.disableSavePosition then
            self.get.disableSavePosition = config.disableSavePosition
        end
        if config.allowMinimize then
            self.get.allowMinimize = config.allowMinimize
        end
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.titleText then
            self.get.titleText = config.titleText
        end
    end,

    onCreate = function(self, config)
        local settings = self.world.func:getSettings()
        local terminalWindowData = settings.terminalWindowData
        
        if not self.get.disableSavePosition then
            if terminalWindowData then
                if terminalWindowData.x and terminalWindowData.y then
                    self:goTo(
                        terminalWindowData.x,
                        terminalWindowData.y
                    )
                end
                if terminalWindowData.width and terminalWindowData.height then
                    self.get.targetWidth = terminalWindowData.width
                    self.get.targetHeight = terminalWindowData.height
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
        
        self.get.paddingLeft = 8
        self.get.paddingRight = 8
        self.get.paddingTop = 22
        self.get.paddingBottom = 4

        self.get.marginLeft = 2
        self.get.marginRight = 2
        self.get.marginTop = 2
        self.get.marginBottom = 2

        self.get.bottomLocked = true

        if self.get.gameProcess then
            self.get.log = self.get.gameProcess.output
        end
        if not self.get.log then
            self.get.log = {}
        end

        self.get.lineSpacing = 2

        self.get.content:createChild("Text", {
            font = "defaultFont",
            text = Localize:get(self.get.titleText),
            color = "#b1d7e9",
            origin = { 0, 0 },
            x = 10, y = 6
        })

        self.get.cont = self.get.content:createChild("Container", {
            x = self.get.paddingLeft,
            y = self.get.paddingTop,
            width = self.get.targetWidth - self.get.paddingLeft - self.get.paddingRight,
            height = self.get.targetHeight - self.get.paddingTop - self.get.paddingBottom,
            origin = 0
        })

        self.get.pool = self.get.cont:createChild("NodePool", {
            x = self.get.cont.left + self.get.marginLeft,
            y = self.get.cont.top + self.get.marginTop,
            ignoreChildren = true
        })
        self.get.activePool = {}
        
        self.get.wallHeight = 0;
        local wrapWidth = self.get.cont.width - self.get.marginLeft - self.get.marginRight

        for i = 1, self.get.poolSize do
            local item = self.get.pool:createChild("Text", {
                font = "defaultFont",
                color = Colors.White,
                origin = 0,
                wrapMode = WrapMode.ByWord
            })
        end

        if self.get.log and #self.get.log > 0 then
            for i = 1, #self.get.log do
                local msg = self.get.log[i]
                self.func:pipeMessage(msg)
            end
        end

        local buttonPosX = 0
        local buttonSpacing = 20

        self.get.exitButton = self.get.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exitOnly",
            y = 4,
            icon = 1,
            props = {
                buttonPosX = buttonPosX
            },
            onUpdate = function(button, deltaTime)
                button.classes.UIButton.func:onUpdate(deltaTime)

                button.x = self.get.targetWidth - button.width - 4 - button.get.buttonPosX
            end,
            onPress = function()
                self.func:closeWindow(function(self)
                    if self.func.onExit then
                        self.func:onExit()
                    end
                    self:destroy()
                end)
                self.get.enabled = false
            end
        })

        if self.get.allowMinimize then
            buttonPosX = buttonPosX + buttonSpacing
            self.get.content:createChild("UIButton", {
                id = "minimizeButton",
                toolTip = "toolTip_minimize",
                y = 4,
                icon = 4,
                props = {
                    buttonPosX = buttonPosX
                },
                onUpdate = function(button, deltaTime)
                    button.classes.UIButton.func:onUpdate(deltaTime)

                    button.x = self.get.targetWidth - button.width - 4 - button.get.buttonPosX
                end,
                onPress = function(self)
                    self.world:minimizeWindow()
                end,
                hotkey = {
                    { Key.LeftAlt, Key.LeftShift, Key.Minus },
                    { Key.RightAlt, Key.RightShift, Key.Minus }
                }
            })
        end

        buttonPosX = buttonPosX + buttonSpacing

        self.frame = terminalWindowData.darkened and 2 or 1
        self.get.darkenButton = self.get.content:createChild("UIButton", {
            id = "darkenButton",
            toolTip = "toolTip_toggleBGOpacity",
            y = 4,
            icon = terminalWindowData.darkened and 15 or 14,
            props = {
                buttonPosX = buttonPosX
            },
            onUpdate = function(button, deltaTime)
                button.classes.UIButton.func:onUpdate(deltaTime)

                button.x = self.get.targetWidth - button.width - 4 - button.get.buttonPosX
            end,
            onPress = function()
                terminalWindowData.darkened = not terminalWindowData.darkened
                self.get.darkenButton.func:setIcon(terminalWindowData.darkened and 15 or 14)

                self.frame = terminalWindowData.darkened and 2 or 1

                self.world.func:saveSettings()
            end
        })

        
        self.get.scrollBar = self.get.content:createChild("FillRect", {
            color = { 70, 70, 70 },
            width = 4,
            origin = 0,
            visible = false,
            alpha = 0.5,
            
            input = {
                active = true,
                onPointerDown = function()
                    self.get.scrollDragged = true
                end,
                onPointerHover = function()
                    self.get.scrollBar.alpha = 0.9
                end,
                onPointerExit = function()
                    if not self.get.scrollDragged then
                        self.get.scrollBar.alpha = 0.6
                    end
                end
            },
            
            onCreate = function(scrollBar)
                scrollBar.get.pos = scrollBar:createChild("FillRect", {
                    color = { 120, 120, 120 },
                    width = scrollBar.width,
                    height = 1,
                    origin = 0
                })
            end,
            
            manageScrollPosition = function(scrollBar)
                scrollBar.visible = true

                scrollBar.x = self.get.cont.x + self.get.cont.width + scrollBar.width - 4
                scrollBar.y = self.get.cont.y + 2
                
                scrollBar.height = self.get.cont.height - 2

                local pos = scrollBar.get.pos
                
                local firstItem = self.get.activePool[1]
                local lastItem = self.get.activePool[#self.get.activePool]

                pos.height = scrollBar.height * ((self.get.cont.height - self.get.marginBottom - self.get.marginTop) / (self.get.wallHeight + self.get.marginBottom + self.get.marginTop - firstItem.y))
                pos.y = -(scrollBar.height - pos.height) * ((-self.get.pool.y + self.get.cont.top + self.get.marginTop - firstItem.y) / ((self.get.cont.bottom - self.get.marginBottom - lastItem.y - lastItem.height) - (self.get.cont.top + self.get.marginTop - firstItem.y)))
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
            item.text = "Error: Invalid assignment or function call."
            item.color = Colors.Red
            self.get.allowTrace = true
            ret = true
        elseif string.starts_with(msg, "Error: ") then
            item.color = Colors.Red
            self.get.allowTrace = true
            ret = true
        elseif string.starts_with(msg, "[json.exception.type_error.302]") then
            local details = string.match(msg, '%[json.exception.type_error.302] type(.*)')
            details = string.gsub(details, "null", "object")
            item.text = "Error: Type" .. details
            item.color = Colors.Red
            self.get.allowTrace = true
            ret = true
        else
            if msg == "Program aborted unexpectedly." then
                item.color = "#5d00ff"
            elseif string.contains(msg, "rendering to window using") then
                item.color = Colors.Cyan
            elseif string.starts_with(msg, "Note:") then
                item.color = Colors.Yellow
            elseif string.starts_with(msg, "Success:") then
                item.color = Colors.Green
            elseif string.starts_with(msg, "Warning:") then
                item.color = Colors.Yellow
            else
                item.color = Colors.White
            end
            ret = false
        end
        item.get.defColor = item.color
        item.get.isError = ret

        local filePath = string.match(msg, '"([^"]+%.lua[c]?)"')
        if filePath and string.len(filePath) > 0 then
            if self.get.projectPath then
                local targetPath = System:join(self.get.projectPath, "lua_scripts", filePath)
                local settings = self.world.func:getSettings()
                if System:exists(targetPath) then
                    item.input:activate()
                    item.input.cursor = Cursor.Pointer
                    item.input:listen("onPointerUp", function(txt)
                        OpenCodeEditor(settings, self.get.projectPath, targetPath)

                        txt.color = txt.get.isError and Colors.White or "#82adc2"
                        txt.tween:to({
                            color = txt.get.defColor,
                            duration = 0.1,
                            onComplete = function(txt)
                                txt.color = txt.get.defColor
                            end
                        })
                    end)
                    item.input:listen("onPointerHover", function(txt)
                        if txt.get.isError then
                            txt.color = "#ff4646"
                        else
                            txt.color = "#c5ecff"
                        end
                        txt.get.defColor = txt.color
                    end)
                    item.input:listen("onPointerExit", function(txt)
                        if txt.get.isError then
                            txt.color = Colors.Red
                        else
                            txt.color = Colors.White
                        end
                        txt.get.defColor = txt.color
                    end)
                else
                    item.input:stopListening("onPointerUp")
                end
            end
        end

        return ret
    end,

    pipeMessage = function(self, msg)
        if string.starts_with(msg, "\t[C") or string.starts_with(msg, "stack tr") then
            return;
        end
        if string.starts_with(msg, "\t[") and not self.get.allowTrace then
            return;
        end
        self.get.allowTrace = false
        
        table.insert(self.get.log, msg)

        local item = self.get.pool:grab()
        
        if not item then
            item = table.remove(self.get.activePool, 1)
        end

        item:activate()
        item.text = msg
        item.input:stopListening()
        item.input:deactivate()

        self.func:checkForError(msg, item)

        local wrapWidth = self.get.cont.width - self.get.marginLeft - self.get.marginRight
        if item.wrapWidth ~= wrapWidth then
            item.wrapWidth = wrapWidth
        end

        if self.get.wallHeight > 0 then
            self.get.wallHeight = self.get.wallHeight + self.get.lineSpacing
        end
        item.y = self.get.wallHeight
        self.get.wallHeight = self.get.wallHeight + item.height

        table.insert(self.get.activePool, item)

        self.get.pool.y = self.get.cont.bottom - self.get.marginBottom - item.y - item.height
        self.get.bottomLocked = true

        item.input:activate()
        item.input.cursor = Cursor.Pointer
        item.input:listen("onRightMouseDown", function(txt)
            System:copyToClipboard(txt.text)
            txt.color = Colors.White;
            txt.tween:to({
                color = txt.get.defColor,
                duration = 0.25
            })
            local copyMsg = self.world:createChild("Text", {
                font = "defaultFont",
                text = "Copied",
                x = self.input.mouse.x,
                y = self.input.mouse.y - 10,
                alpha = 0
            })
            copyMsg.tween:to({
                alpha = 1,
                duration = 0.25,
                onComplete = function(copyMsg)
                    copyMsg.tween:to({
                        alpha = 0,
                        duration = 0.25,
                        onComplete = function(copyMsg)
                            copyMsg:destroy()
                        end
                    })
                end
            })
        end)

        return item
    end,

    handleMessage = function(self, msg)
        return self.func:pipeMessage(msg)
    end,

    savePosition = function(self)
        if not self.get.disableSavePosition then
            local setting = self.world.func:getSettings()
            if not setting.terminalWindowData then
                setting.terminalWindowData = {}
            end

            setting.terminalWindowData.x = self.x
            setting.terminalWindowData.y = self.y

            setting.terminalWindowData.width = self.get.targetWidth
            setting.terminalWindowData.height = self.get.targetHeight

            self.world.func:saveSettings()
        end
    end,

    onSizeChange = function(self)
        local wrapWidth = self.get.cont.width - self.get.marginLeft - self.get.marginRight
        local lastitem = nil
        self.get.wallHeight = 0
        if #self.get.activePool > 0 then
            for i = 1, #self.get.activePool do
                local item = self.get.activePool[i]

                if item.wrapWidth ~= wrapWidth then
                    item.wrapWidth = wrapWidth
                end

                if lastitem then
                    item.y = lastitem.y + lastitem.height + self.get.lineSpacing
                else
                    item.y = 0
                end
                lastitem = item
            end
            self.get.wallHeight = lastitem.y + lastitem.height
        end
    end,

    onUpdate = function(self)
        self.classes.UIWindow.func:onUpdate(self)

        if self.input.hovered then
            self.get.pool.y = self.get.pool.y + self.input.mouse.wheel.y * 5

            if self.input.mouse.wheel.y ~= 0 and self.get.wallHeight > (self.get.cont.height - self.get.marginBottom - self.get.marginTop) then
                self.get.bottomLocked = false
            end
        end
        
        if self.get.scrollDragged then
            if self.input.pointer.isDown then
                self.get.scrollBar.alpha = 0.9
                local firstItem = self.get.activePool[1]
                local lastItem = self.get.activePool[#self.get.activePool]
                
                local per = (self.input.pointer.y - self.get.scrollBar.worldPos.y) / self.get.scrollBar.height
                local top = self.get.cont.top + self.get.marginTop - firstItem.y
                local bottom = self.get.cont.bottom - self.get.marginBottom - lastItem.y - lastItem.height
                
                self.get.pool.y = top + per * (bottom - top)
                
                if self.get.wallHeight > (self.get.cont.height - self.get.marginBottom - self.get.marginTop) then
                    self.get.bottomLocked = false
                end
            else
                self.get.scrollDragged = false
                self.get.scrollBar.alpha = 0.6
            end
        end

        self.get.cont.width = self.get.targetWidth - self.get.paddingLeft - self.get.paddingRight
        self.get.cont.height = self.get.targetHeight - self.get.paddingTop - self.get.paddingBottom
        self.get.pool.x = self.get.cont.left + self.get.marginLeft

        if #self.get.activePool > 0 then
            local firstItem = self.get.activePool[1]
            local lastItem = self.get.activePool[#self.get.activePool]
            local viewableHeight = self.get.cont.height - self.get.marginBottom - self.get.marginTop

            if self.get.wallHeight <= viewableHeight then
                self.get.pool.y = self.get.cont.top + self.get.marginTop - firstItem.y
                self.get.bottomLocked = true
            else
                if self.get.bottomLocked then
                    self.get.pool.y = self.get.cont.bottom - self.get.marginBottom - lastItem.y - lastItem.height
                else
                    if self.get.pool.y + firstItem.y > self.get.cont.top + self.get.marginTop then
                        self.get.pool.y = self.get.cont.top + self.get.marginTop - firstItem.y
                    end
                    if self.get.pool.y + lastItem.y + lastItem.height < self.get.cont.bottom - self.get.marginBottom then
                        self.get.pool.y = self.get.cont.bottom - self.get.marginBottom - lastItem.y - lastItem.height
                        self.get.bottomLocked = true
                    end
                end
            end
        end
        
        if self.get.wallHeight > (self.get.cont.height - self.get.marginBottom - self.get.marginTop) then
            self.get.scrollBar.func:manageScrollPosition()
        else
            self.get.scrollBar.visible = false
        end
    end,

    unbindGameProcess = function(self)
        self.get.gameProcess = nil
    end,

    startLoading = function(self)
        self.get.loadingBar = self.get.content:createChild("LoadingBar", {
            alpha = 0.3,
            x = 2, y = self.get.targetHeight - 4,
            width = self.get.targetWidth - 4
        })
    end,

    stopLoading = function(self)
        if self.get.loadingBar then
            self.get.loadingBar.tween:to({
                alpha = 0,
                duration = 0.25,
                onComplete = function(loadingBar)
                    loadingBar:destroy()
                end
            })
            self.get.loadingBar = nil
        end
    end
})