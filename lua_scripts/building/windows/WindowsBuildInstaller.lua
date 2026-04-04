Nodes:define("WindowsBuildInstaller", "UIWindow", {
    width = 270,
    height = 120,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.super.UIWindow.func:onCreate()

        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("label_windowsBuilderNotFound"),
            color = Colors.Yellow,
            origin = 0,
            input = true
        })

        self.get.msgTxt = self.get.content:createChild("Text", {
            x = 10, y = 24,
            font = "defaultFont",
            text = Localize:get("label_windowsBuilderNotice"),
            color = Colors.White,
            origin = 0,
            input = true
        })
        
        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:closeWindow(function()
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })
                    
                    self:destroy()
                end)
            end
        })

        self.get.pathField = self.get.content:createChild("TextField", {
            x = 8,
            y = self.get.msgTxt.y + self.get.msgTxt.height + 6,
            width = self.get.targetWidth - 16 - 18,
            inputEnabled = false,
            defaultText = Localize:get("label_windowsModuleFile")
        })

        self.get.browseButton = self.get.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseFile",
            x = self.get.pathField.x + self.get.pathField.width + 4,
            y = self.get.pathField.y,
            icon = 6,
            onPress = function()
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    self.get.modulePath = nil

                    local path = System:browseFile(self.get.projectPath)

                    self.world:showWindow()
                    
                    if string.len(path) == 0 then
                        return
                    end

                    self.get.pathField.func:setText("")
                    if self.func:checkModule(path) then
                        self.get.modulePath = path
                        self.get.pathField.func:setText(self.func:truncatePath(path))
                    end
                end)
            end
        })

        self.get.errorMessage = self.get.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 72
        })

        local buildButton = self.get.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                if self.func:checkModule(self.get.modulePath) then
                    self.func:continueBuilding()
                end
            end
        })
        buildButton.x = self.get.targetWidth - buildButton.width - 8
        buildButton.y = self.get.targetHeight - buildButton.height - 6

        local downloadButton = self.get.content:createChild("UIButton", {
            id = "downloadButton",
            text = "label_downloadModule",
            onPress = function()
                self.func:downloadModule()
            end
        })
        downloadButton.x = 8
        downloadButton.y = buildButton.y
    end,

    checkModule = function(self, path)
        if not path then
            self.get.errorMessage:setText(Localize:get("error_expectedAmara2BuildModule"))
            self.get.errorMessage.visible = true
            return false
        end
        if not System:exists(path) then
            self.get.errorMessage:setText(Localize:get("error_windowsModuleNotFound"))
            self.get.errorMessage.visible = true
            return false
        end
        if not (System:getFileName(path) == "amara2_windows_build_module.zip") then
            self.get.errorMessage:setText(Localize:get("error_expectedAmara2BuildModule"))
            self.get.errorMessage.visible = true
            return false
        end
        self.get.errorMessage.visible = false
        return true
    end,

    truncatePath = function(self, _path)
        local txt = self.get.pathField.get.txt
        local str = _path
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.get.pathField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end,

    downloadModule = function(self)
        System:openWebsite("https://github.com/BigBossErndog/Amara2-Build-Modules/releases")
    end,

    continueBuilding = function(self)
        local installerNode = self
        self.func:closeWindow(function(win)
            local printLog = self.world.get.windows:createChild("TerminalWindow", {
                props = {
                    projectPath = self.get.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,

                onCreate = function(self)
                    self.super.TerminalWindow.func:onCreate()
                    self.func:startLoading()

                    self.get.gameProcess = self:createChild("ProcessNode", {
                        arguments = {
                            Game.executable,
                            "-context", System:getBasePath(),
                            "-script", System:getScriptPath("building/windows/WindowsBuildExtractor"),
                            "-buildmodule", installerNode.get.modulePath
                        },
                        onOutput = function(process, msg)
                            self.func:handleMessage(msg)
                        end,
                        onExit = function(process, exitCode)
                            self.func:stopLoading()
                            if exitCode == 0 and System:exists(System:getRelativePath("build_modules/amara2_windows_build_module/clang-llvm/bin/clang.exe")) then
                                self.get.success = true                          
                            end

                            self.func:unbindGameProcess()
                        end
                    })
                end,

                onExit = function(self)
                    if self.get.gameProcess then
                        self.get.gameProcess:destroy()
                        self.get.gameProcess = nil
                    end
                    if not self.get.success then
                        local newWindow = self.world.get.windows:createChild("ProjectWindow", {
                            projectPath = self.get.projectPath
                        })
                    else
                        local newWindow = self.world.get.windows:createChild("BuildPlatformMenu", {
                            projectPath = self.get.projectPath
                        })
                        newWindow.func:openWindow()
                    end
                end
            })
            printLog.func:openWindow()
            
            self:destroy()
        end)
    end
})