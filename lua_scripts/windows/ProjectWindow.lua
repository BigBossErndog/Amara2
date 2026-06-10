Nodes:define("ProjectWindow", "UIWindow", {
    width = 108,
    height = 60,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.playOnStart then
            self.get.playOnStart = config.playOnStart
        end
    end,
    
    onCreate = function(self)
        self.super.UIWindow.func:onCreate()

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        local projectName = projectData["project-name"]

        if projectData.exampleProject then
            self.get.exampleProject = true
        end

        local cont = self.get.content:createChild("Container", {
            width = self.get.targetWidth - 6,
            height = self.get.targetHeight - 4,
            origin = 0,
            x = 3, y = 2
        })

        local title = cont:createChild("Text", {
            y = cont.top + 2,
            font = "defaultFont",
            text = projectName,
            color = Colors.White,
            origin = { 0, 0 }
        })
        title.x = -math.floor(title.width/2.0)
        
        if title.width > self.get.targetWidth - 4 then
            title.x = cont.left + 3
            title.tween:to({
                x = cont.right - 3 - title.width,
                duration = 4,
                ease = Ease.SineInOut,
                yoyo = true,
                repeats = -1,
                delay = 2,
                interim = 2
            })
        end

        local buttonPos = Vector2.new(6, 18)
        local buttonSpacing = 20

        self.get.playButton = self.get.content:createChild("UIButton", {
            id = "playButton",
            toolTip = "toolTip_runGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 3,
            hotkey = {
                left = { Key.LeftAlt, Key.LeftShift, Key.Enter },
                right = { Key.RightAlt, Key.RightShift, Key.Enter }
            },
            debugging = true,
            onPress = function()
                if not self.get.gameProcess then
                    self.func:runGame()
                else 
                    self.func:stopGame()
                end
            end,
            onRightClick = function()
                if self.get.gameProcess then
                    self.func:stopGame()
                end
                if self.get.printLog then
                    self.get.printLog.func:unbindGameProcess()
                    self.get.printLog.func:closeWindow(function(self)
                        self:destroy()
                    end)
                    self.get.printLog = nil
                end
                self.func:closeWindow(function(button)
                    button.get.enabled = false
                    
                    local newWindow = self.parent:createChild("RunArgumentsWindow", {
                        projectPath = self.get.projectPath,
                        exampleProject = self.get.exampleProject
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.codeEditorButton = self.get.content:createChild("CodeEditorButton", {
            id = "openCodeEditorButton",
            toolTip = "toolTip_openCodeEditor",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 7,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.X },
                { Key.RightAlt, Key.RightShift, Key.X }
            }
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.openDirectoryButton = self.get.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openProjectDirectory",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 6,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.P },
                { Key.RightAlt, Key.RightShift, Key.P }
            },
            onPress = function()
                System:openDirectory(self.get.projectPath)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "minimizeButton",
            toolTip = "toolTip_minimize",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 4,
            onPress = function(button)
                self.world:minimizeWindow()
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Minus },
                { Key.RightAlt, Key.RightShift, Key.Minus }
            }
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 5,
            onPress = function()
                if self.get.gameProcess then
                    self.func:stopGame()
                end
                if self.get.printLog then
                    self.get.printLog.func:unbindGameProcess()
                    self.get.printLog.func:closeWindow(function(self)
                        self:destroy()
                    end)
                    self.get.printLog = nil
                end
                self.func:closeWindow(function(button)
                    button.get.enabled = false
                    
                    local newWindow
                    if self.get.exampleProject then
                        newWindow = self.parent:createChild("ExamplesWindow")
                    else
                        newWindow = self.parent:createChild("MainWindow")
                    end
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Backspace },
                { Key.RightAlt, Key.RightShift, Key.Backspace }
            }
        })

        buttonPos.x = 6
        buttonPos.y = buttonPos.y + buttonSpacing
        
        self.get.buildButton = self.get.content:createChild("UIButton", {
            id = "buildButton",
            toolTip = "toolTip_buildGame",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 8,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.B },
                { Key.RightAlt, Key.RightShift, Key.B }
            },
            onPress = function(button)
                button.get.enabled = false
                self.func:buildGame()
            end,
            onRightClick = function(button)
                button.get.enabled = false
                self.func:buildGame(true)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.printLogButton = self.get.content:createChild("UIButton", {
            id = "printLogButton",
            toolTip = "toolTip_openPrintLog",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 11,
            onPress = function()
                if not self.get.printLog then
                    self.get.printLog = self.parent:createChild("TerminalWindow", {
                        projectPath = self.get.projectPath,
                        gameProcess = self.get.gameProcess,
                        onExit = function()
                            self.get.printLog = nil
                            self.get.printLogButton.func:setIcon(11)
                        end
                    })
                    self.get.printLog.func:openWindow()
                else
                    self.get.printLog.get.exitButton.func:forcePress()
                end
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.M },
                { Key.RightAlt, Key.RightShift, Key.M }
            }
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "openDocsButton",
            toolTip = "toolTip_openDocs",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 19,
            onPress = function()
                System:openWebsite("https://github.com/BigBossErndog/Amara2/wiki")
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "projectSettingsButton",
            toolTip = "toolTip_projectSettings",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 23,
            onPress = function()
                if self.get.gameProcess then
                    self.func:stopGame()
                end
                if self.get.printLog then
                    self.get.printLog.func:unbindGameProcess()
                    self.get.printLog.func:closeWindow()
                    self.get.printLog = nil
                end
                self.func:closeWindow(function(button)
                    button.get.enabled = false
                    
                    local newWindow = self.parent:createChild("ProjectSettingsWindow", {
                        projectPath = self.get.projectPath,
                        exampleProject = self.get.exampleProject
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        buttonPos.x = buttonPos.x + buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "moreToolsButton",
            toolTip = "toolTip_moreTools",
            x = buttonPos.x,
            y = buttonPos.y,
            icon = 17,
            onPress = function()
                
            end
        })

        local projectWindowData = self.world.func:getSettings().projectWindowData

        if self.get.resumePosition and projectWindowData then
            self:goTo(
                projectWindowData.x,
                projectWindowData.y
            )
        end
        self.func:openWindow(function(self)
            if not self.get.resumePosition then
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

        if not self.get.exampleProject then
            self.world.func:registerProject(self.get.projectPath)
        end

        if self.get.playOnStart then
            self.get.playButton.func:onPress()
        end
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
        System:openDirectory(self.get.projectPath)
    end,
    
    openCodeEditor = function(self)
        self.get.codeEditorButton.func:onPress()
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
        local testBuild = nil
        if Game.platform == "windows" then
            testBuild = System:join(self.get.projectPath, "build", "test", "Amara2.exe")
            if System:exists(testBuild) then
                exe = testBuild
            else
                testBuild = nil
            end
        end
        
        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        
        local args = {
            exe,
            "-context", self.get.projectPath,
            "-debugging",
            "-script", "index.lua",
            "-script", System:getScriptPath("utility/BringGameToFront.lua")
        }
        if projectData.test_arguments then
            for key, value in pairs(projectData.test_arguments) do
                if type(value) ~= "boolean" then
                    table.insert(args, "-" .. key)
                    table.insert(args, tostring(value))
                else
                    table.insert(args, "-D" .. key)
                end
            end
        end

        self.get.gameProcess = self:createChild("ProcessNode", {
            arguments = args,
            environment = testBuild and System:getDirectoryOf(testBuild),
            onOutput = function(process, msg)
                if self.get.printLog then
                    self.get.printLog.func:handleMessage(msg)
                end
            end,
            onExit = function(process, exitCode, errorMessage)
                if self.get.printLog then
                    self.get.printLog.func:unbindGameProcess()
                elseif exitCode ~= 0 then
                    self.get.printLogButton.func:forcePress()
                end

                if exitCode == -1 then
                    self.get.printLog.func:handleMessage(Localize:get("error_failedToRunGame"))
                end
                if errorMessage then
                    self.get.printLog.func:handleMessage(errorMessage)
                end
                if exitCode ~= 0 then
                    self.get.printLog.func:handleMessage("Program aborted unexpectedly.")
                end

                self.get.gameProcess = nil
                self.get.playButton.func:setIcon(3)
            end
        })

        self.get.playButton.func:setIcon(13)
    end,

    stopGame = function(self)
        if self.get.gameProcess then
            self.get.gameProcess:destroy()
            self.get.gameProcess = nil
        end

        self.get.playButton.func:setIcon(3)
    end,

    buildGame = function(self, buildTest)
        self.func:stopGame()
        
        self.world.get.windows.func:closeAll(function(window)
            window:destroy()
        end)

        if Game.platform == "windows" then
            local settings = self.world.func:getSettings()
            
            if not System:exists(System:getRelativePath("build_modules/amara2_windows_build_module/clang-llvm/bin/clang.exe")) then
                local newWindow = self.world.get.windows:createChild("WindowsBuildInstaller", {
                    projectPath = self.get.projectPath,
                    exampleProject = self.get.exampleProject,
                    buildTest = buildTest
                })
                newWindow.func:openWindow()
            elseif buildTest then
                local newWindow

                if (not settings.vsBuildToolsInstalled) and (not System:VSBuildToolsInstalled()) then
                    newWindow = self.world.get.windows:createChild("VSBuildToolsInstaller", {
                        projectPath = self.get.projectPath,
                        buildTest = true
                    })
                    newWindow.func:openWindow()
                else
                    if not settings.vsBuildToolsInstalled then
                        settings.vsBuildToolsInstalled = true
                    end
                    newWindow = self.world.get.windows:createChild("WindowsBuildOptions", {
                        projectPath = self.get.projectPath,
                        buildTest = true
                    })
                    newWindow.func:openWindow()
                end

            else
                local newWindow = self.world.get.windows:createChild("BuildPlatformMenu", {
                    projectPath = self.get.projectPath,
                    exampleProject = self.get.exampleProject
                })
                newWindow.func:openWindow()
            end
        end
    end
})