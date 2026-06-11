Nodes:define("VSBuildToolsInstaller", "UIWindow", {
    width = 256,
    height = 100,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.buildTest then
            self.get.buildTest = config.buildTest
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

        local buildButton = self.get.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                if System:VSBuildToolsInstalled() then
                    self.func:continueBuilding()
                else
                    self.func:closeWindow(function(win)
                        local newWindow = self.world.get.windows:createChild("VSBuildToolsInstaller", {
                            projectPath = self.get.projectPath
                        })
                        newWindow.func:openWindow()
                        self:destroy()
                    end)
                end
            end
        })
        buildButton.x = self.get.targetWidth - buildButton.width - 8
        buildButton.y = self.get.targetHeight - buildButton.height - 6

        local downloadButton = self.get.content:createChild("UIButton", {
            id = "downloadButton",
            text = Localize("label_installVSBuildTools"),
            onPress = function()
                self.func:downloadVSBuildTools()
            end
        })
        downloadButton.x = buildButton.x - downloadButton.width - 8
        downloadButton.y = buildButton.y
    end,

    continueBuilding = function(self)
        self.func:closeWindow(function(win)
            local newWindow = self.world.get.windows:createChild("WindowsBuildOptions", {
                projectPath = self.get.projectPath,
                buildTest = self.get.buildTest
            })
            newWindow.func:openWindow()

            self:destroy()
        end)
    end,

    downloadVSBuildTools = function(self)
        System:openWebsite("https://visualstudio.microsoft.com/visual-cpp-build-tools/")
    end
})