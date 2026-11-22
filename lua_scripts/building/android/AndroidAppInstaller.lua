Nodes:define("AndroidInstallOrRebuild", "UIWindow", {
    width = 270,
    height = 120,

    onConfigure = function(self, config)
        table.assign(self.get, {
            projectPath = config.projectPath,
            projectData = config.projectData,
            exampleProject = config.exampleProject
        })
    end,

    onCreate = function(self, config)
        self.super.UIWindow.func:onCreate()

        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_buildAlreadyExists"),
            color = Colors.Yellow,
            origin = 0,
            input = true
        })

        self.get.msgTxt = self.get.content:createChild("Text", {
            x = 10, y = 24,
            wrapWidth = self.get.targetWidth - 20,
            font = "defaultFont",
            text = Localize:get("label_androidBuildAlreadyExists"),
            color = Colors.White,
            origin = 0
        })

        local buttonPos = self.get.targetWidth - 22
        
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
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        local rebuildButton = self.get.content:createChild("UIButton", {
            id = "rebuildButton",
            text = "label_continueAndRebuild",
            onPress = function()
                self.func:closeWindow(function()
                    self.world:createChild("AndroidBuildNode", {
                        projectPath = self.get.projectPath,
                        projectData = self.get.projectData,
                        exampleProject = self.get.exampleProject
                    })
                    self:destroy()
                end)
            end
        })
        rebuildButton.x = self.get.targetWidth - rebuildButton.width - 8
        rebuildButton.y = self.get.targetHeight - rebuildButton.height - 6

        local installButton = self.get.content:createChild("UIButton", {
            id = "installButton",
            text = "label_skipAndInstall",
            onPress = function()
                self.func:closeWindow(function()
                    self.world:createChild("AndroidADBInstaller", {
                        projectPath = self.get.projectPath,
                        projectData = self.get.projectData,
                        exampleProject = self.get.exampleProject
                    })
                    self:destroy()
                end)
            end
        })
        installButton.x = rebuildButton.x - installButton.width - 10
        installButton.y = rebuildButton.y
    end
})

Nodes:define("AndroidADBInstaller", "Action", {
    onConfigure = function(self, config)
        table.assign(self.get, {
            sdk = config.sdk,
            buildDir = config.buildDir,
            projectPath = config.projectPath,
            projectData = config.projectData,
            printLog = config.printLog,
            errorOutputPath = config.errorOutputPath
        })
    end,

    onCreate = function(self)
        if not self.get.sdk then
            self.get.sdk = System:locateAndroidSDK()
        end
        if not self.get.buildDir and self.get.projectPath then
            self.get.buildDir = System:join(self.get.projectPath, "build", "android")
        end
        if not self.get.errorOutputPath then
            self.get.errorOutputPath = System:join(self.get.buildDir, "build_error.txt")
        end
        if not self.get.printLog then
            self.get.printLog = self.world.get.windows:createChild("TerminalWindow", {
                titleText = "title_androidBuilder",
                gameProcess = self,
                props = {
                    projectPath = self.get.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,
                onExit = function(self)
                    if self.get.gameProcess then
                        System:remove(self.get.gameProcess.get.batchFilePath)
                        for _, build in ipairs(self.get.gameProcess.get.builds) do
                            System:remove(build.file)
                        end
                    end

                    self.world.get.windows:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })

                    if self.get.gameProcess then
                        self.get.gameProcess:destroy()
                        self.get.gameProcess = nil
                    end
                end
            })
            self.get.printLog.func:openWindow()
            self.world:showWindow()
        end

        self.get.printLog.func:handleMessage(Localize:get("label_installingAppToAndroidDevice"))
        local adb = self.get.sdk["adb"]

        local apk_name = self.get.projectData.android["app-name"] .. " " .. (self.get.projectData.android["version-name"] or "1.0.0") .. ".apk"
        local apk_path = System:join(self.get.buildDir, apk_name)
        local errorCommand = " >> " .. fix_path(self.get.errorOutputPath) .. " 2>&1 || exit 1"
        local command = quote_if_needed(adb) .. " install " .. quote_if_needed(apk_path) .. errorCommand

        self.get.printLog.func:handleMessage(apk_name)

        local batchFilePath = System:join(self.get.buildDir, "install_app.bat")
        System:writeFile(batchFilePath, command)
        
        local systemCommand = "System:exit(System:executeTerminal(" .. string.format("%q", quote_if_needed(batchFilePath)) .. "))"

        self.get.printLog.func:startLoading()
        self.get.install_process = self.world:createChild("ProcessNode", {
            arguments = {
                Game.executable,
                "-context", System:getBasePath(),
                "-inline-script", systemCommand,
                "-inline-override"
            },
            onExit = function(node, exitCode)
                if System:exists(batchFilePath) then
                    System:remove(batchFilePath)
                end
                if exitCode == 0 then
                    if System:exists(self.get.errorOutputPath) then
                        System:remove(self.get.errorOutputPath)
                    end
                    self.get.printLog.func:handleMessage(Localize:get("label_successfullyInstalledApp"))
                else
                    if System:exists(self.get.errorOutputPath) then
                        local error_message = System:readFile(self.get.errorOutputPath)
                        self.get.printLog.func:handleMessage("Error: Build failed.\n" .. error_message)
                        System:remove(self.get.errorOutputPath)
                    end
                end
                self.get.printLog.func:unbindGameProcess()
                self.get.printLog.func:stopLoading()

                self:complete()
            end
        })
        self.get.printLog.get.gameProcess = self.get.install_process
    end
})