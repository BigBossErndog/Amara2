Nodes:define("WindowsBuildInstaller", "UIWindow", {
    width = 256,
    height = 120,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.props.title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("label_windowsBuilderNotFound"),
            color = Colors.Red,
            origin = 0,
            input = true
        })

        self.props.msgTxt = self.props.content:createChild("Text", {
            x = 10, y = 24,
            font = "defaultFont",
            text = Localize:get("label_windowsBuilderNotice"),
            color = Colors.White,
            origin = 0,
            input = true
        })
        
        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.props.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        self.props.pathField = self.props.content:createChild("TextField", {
            x = 8,
            y = self.props.msgTxt.y + self.props.msgTxt.height + 6,
            width = self.props.targetWidth - 16 - 18,
            inputEnabled = false,
            defaultText = Localize:get("label_windowsModuleFile")
        })

        self.props.browseButton = self.props.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseFile",
            x = self.props.pathField.x + self.props.pathField.width + 4,
            y = self.props.pathField.y,
            icon = 6,
            onPress = function()
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    self.props.modulePath = nil

                    local path = System:browseFile(self.props.projectPath)

                    self.world:showWindow()
                    
                    if string.len(path) == 0 then
                        return
                    end

                    self.props.pathField.func:setText("")
                    if self.func:checkModule(path) then
                        self.props.modulePath = path
                        self.props.pathField.func:setText(self.func:truncatePath(path))
                    end
                end)
            end
        })

        self.props.errorMessage = self.props.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 72
        })

        local buildButton = self.props.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                if self.func:checkModule(self.props.modulePath) then
                    self.func:continueBuilding()
                end
            end
        })
        buildButton.x = self.props.targetWidth - buildButton.width - 8
        buildButton.y = self.props.targetHeight - buildButton.height - 6

        local downloadButton = self.props.content:createChild("UIButton", {
            id = "downloadButton",
            text = "label_downloadModule",
            onPress = function()
                self.func:downloadModule()
            end
        })
        downloadButton.x = buildButton.x - downloadButton.width - 8
        downloadButton.y = buildButton.y
    end,

    checkModule = function(self, path)
        if not path then
            return false
        end
        if not System:exists(path) then
            return false
        end
        if not (System:getFileName(path) == "amara2_windows_build_module.zip") then
            return false
        end
        return true
    end,

    truncatePath = function(self, _path)
        local txt = self.props.pathField.props.txt
        local str = _path
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.props.pathField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end,

    downloadModule = function(self)
        System:openWebsite("https://github.com/BigBossErndog/Amara2-Build-Modules/releases")
    end,

    continueBuilding = function(self)
        self.func:closeWindow(function(win)
            local printLog = self.world.props.windows:createChild("TerminalWindow", {
                props = {
                    projectPath = self.props.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,

                onCreate = function(self)
                    self.classes.TerminalWindow.func:onCreate()
                    
                    self.props.gameProcess = self:createChild("ProcessNode", {
                        arguments = {
                            Game.executable,
                            "-buildmodule", self.props.modulePath,
                            "-script", System:getScriptPath("building/windows/WindowsBuildExtractor")
                        },
                        onOutput = function(process, msg)
                            print(msg)
                            self.func:handleMessage(msg)
                        end,
                        onExit = function()

                        end
                    })
                end,

                onExit = function(self)
                    local newWindow = self.world.props.windows:createChild("ProjectWindow", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                end
            })
            printLog.func:openWindow()
            
            self:destroy()
        end)
    end
})