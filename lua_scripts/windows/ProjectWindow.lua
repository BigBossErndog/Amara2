return NodeFactory:create("ProjectWindow", "UIWindow", {
    width = 128,
    height = 40,

    onConfigure = function(self, config)
        self:super_configure(config)

        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,
    
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
        self.world.alwaysOnTop = true

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

        self.props.content:createChild("UIButton", {
            id = "playButton",
            toolTip = "toolTip_runGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 2,
            onPress = function()
                self.func:runGame()
            end
        })
        
        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.A },
            onPress = function()
                self.func:runGame()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "buildButton",
            toolTip = "toolTip_buildGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 7,
            onPress = function()
                
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.codeEditorButton = self.props.content:createChild("CodeEditorButton", {
            id = "openCodeEditorButton",
            toolTip = "toolTip_openCodeEditor",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 6
        })

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.E },
            onPress = function()
                self.func:openCodeEditor()
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openDirectory_shortcut",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 5,
            onPress = function()
                System:openDirectory(self.props.projectPath)
            end
        })

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.F },
            onPress = function()
                System:openDirectory(self.props.projectPath)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_back",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 4,
            onPress = function()
                self.func:closeWindow(function()
                    self.props.enabled = false
                    
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
            icon = 0,
            onPress = function()
                self.func:closeWindow(function(self)
                    self.world:destroy()
                end)
                self.props.enabled = false
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

        if settings.codeEditor then
            self.func:openCodeEditor()
        else
            self.func:openDirectory()
        end
    end,

    runGame = function(self)
        
    end
})