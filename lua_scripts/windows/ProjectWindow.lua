return Nodes:define("ProjectWindow", "UIWindow", {
    width = 108,
    height = 60,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,
    
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))
        local projectName = projectData["project-name"]

        local title = self.props.content:createChild("Text", {
            y = 4,
            font = "defaultFont",
            text = projectName,
            color = Colors.White,
            origin = { 0, 0 }
        })
        title.x = math.floor(self.props.targetWidth/2.0 - title.width/2.0)

        local buttonPos = Vector2.new(6, 18)
        local buttonSpacing = 20

        self.props.playButton = self.props.content:createChild("UIButton", {
            id = "playButton",
            toolTip = "toolTip_runGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 3,
            onPress = function()
                if not self.props.gameProcess then
                    self.func:runGame()
                else 
                    self.func:stopGame()
                end
            end
        })
        
        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.A },
            onPress = function()
                self.props.playButton.func:forcePress()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.codeEditorButton = self.props.content:createChild("CodeEditorButton", {
            id = "openCodeEditorButton",
            toolTip = "toolTip_openCodeEditor",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 7
        })

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.E },
            onPress = function()
                self.props.codeEditorButton.func:forcePress()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.openDirectoryButton = self.props.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openProjectDirectory",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 6,
            onPress = function()
                System:openDirectory(self.props.projectPath)
            end
        })

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.P },
            onPress = function()
                self.props.openDirectoryButton.func:forcePress()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 5,
            onPress = function()
                if self.props.gameProcess then
                    self.func:stopGame()
                end
                if self.props.printLog then
                    self.props.printLog.func:unbindGameProcess()
                    self.props.printLog.func:closeWindow()
                    self.props.printLog = nil
                end
                self.func:closeWindow(function(button)
                    button.props.enabled = false
                    
                    local newWindow = self.parent:createChild("MainWindow")
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 1,
            onPress = function(button)
                self.world.props.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                button.props.enabled = false
            end
        })

        buttonPos.x = 6
        buttonPos.y = buttonPos.y + buttonSpacing
        
        self.props.content:createChild("UIButton", {
            id = "buildButton",
            toolTip = "toolTip_buildGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 8,
            onPress = function(button)
                button.props.enabled = false
                self.func:buildGame()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.printLogButton = self.props.content:createChild("UIButton", {
            id = "printLogButton",
            toolTip = "toolTip_openPrintLog",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 11,
            onPress = function()
                if not self.props.printLog then
                    self.props.printLog = self.parent:createChild("TerminalWindow", {
                        projectPath = self.props.projectPath,
                        gameProcess = self.props.gameProcess,
                        onExit = function()
                            self.props.printLog = nil
                            self.props.printLogButton.func:setIcon(11)
                        end
                    })
                    self.props.printLog.func:openWindow()
                else
                    self.props.printLog.props.exitButton.func:forcePress()
                end
            end
        })

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.M },
            onPress = function()
                self.props.printLogButton.func:forcePress()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "copyProjectButton",
            toolTip = "toolTip_copyProject",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 12,
            onPress = function()
                if self.props.gameProcess then
                    self.func:stopGame()
                end
                if self.props.printLog then
                    self.props.printLog.func:unbindGameProcess()
                    self.props.printLog.func:closeWindow()
                    self.props.printLog = nil
                end
                self.func:closeWindow(function(button)
                    button.props.enabled = false
                    
                    local newWindow = self.parent:createChild("CopyProjectWindow", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "openDocsButton",
            toolTip = "toolTip_openDocs",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 19,
            onPress = function()
                
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "moreToolsButton",
            toolTip = "toolTip_moreTools",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 17,
            onPress = function()
                
            end
        })

        local projectWindowData = self.world.func:getSettings().projectWindowData

        if self.props.resumePosition and projectWindowData then
            self:goTo(
                projectWindowData.x,
                projectWindowData.y
            )
        end
        self.func:openWindow(function(self)
            if not self.props.resumePosition then
                if projectWindowData then
                    self.tween:to({
                        x = projectWindowData.x,
                        y = projectWindowData.y,
                        duration = 0.2,
                        ease = Ease.SineInOut
                    })
                else
                    self.tween:to({
                        x = self.world.right - self.width/2 - 16,
                        y = self.world.bottom - self.height/2 - 24,
                        duration = 0.2,
                        ease = Ease.SineInOut,
                        onComplete = function(self)
                            self.func:savePosition()
                        end
                    })
                end
            end
        end)

        self.input:listen("onPointerUp", function(self)
            self.func:savePosition()
        end)

        self.world.func:registerProject(self.props.projectPath)
    end,

    savePosition = function(self)
        local setting = self.world.func:getSettings()
        if not setting.projectWindowData then
            setting.projectWindowData = {}
        end
        setting.projectWindowData.x = self.x
        setting.projectWindowData.y = self.y

        self.world.func:saveSettings()
    end,

    openDirectory = function(self)
        System:openDirectory(self.props.projectPath)
    end,

    openCodeEditor = function(self)
        self.props.codeEditorButton.func:onPress()
    end,

    openDefault = function(self)
        local settings = self.world.func:getSettings()

        if settings.autoOpenCodeEditor then
            if settings.codeEditor then
                self.func:openCodeEditor()
            else
                self.func:openDirectory()
            end
        end
    end,

    runGame = function(self)
        self.func:stopGame()

        local exe = Game.executable

        self.props.gameProcess = self:createChild("ProcessNode", {
            arguments = {
                exe,
                "-context",
                self.props.projectPath
            },
            onOutput = function(process, msg)
                if self.props.printLog then
                    self.props.printLog.func:handleMessage(msg)
                end
            end,
            onExit = function(process, exitCode, errorMessage)
                if self.props.printLog then
                    self.props.printLog.func:unbindGameProcess()
                elseif exitCode ~= 0 then
                    self.props.printLogButton.func:forcePress()
                end
                if exitCode == -1 then
                    self.props.printLog.func:handleMessage(Localize:get("error_failedToRunGame"))
                end
                if errorMessage then
                    self.props.printLog.func:handleMessage(errorMessage)
                end
                self.props.gameProcess = nil
                self.props.playButton.func:setIcon(3)
            end
        })

        self.props.playButton.func:setIcon(13)
    end,

    stopGame = function(self)
        if self.props.gameProcess then
            self.props.gameProcess:destroy()
            self.props.gameProcess = nil
        end

        self.props.playButton.func:setIcon(3)
    end,

    buildGame = function(self)
        self.func:stopGame()
        
        self.world.props.windows.func:closeAll(function(window)
            window:destroy()
        end)

        local newWindow = self.world.props.windows:createChild("BuildOptions", {
            projectPath = self.props.projectPath
        })
        newWindow.func:openWindow()
    end
})