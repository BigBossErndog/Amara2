Nodes:define("BuildPlatformMenu", "UIWindow", {
    width = 200,
    height = 82,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.get.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_choosePlatform"),
            color = Colors.White,
            origin = 0
        })

        local buildButton = self.get.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                self.func:continueBuilding()
            end
        })
        buildButton.x = self.get.targetWidth - buildButton.width - 8
        buildButton.y = self.get.targetHeight - buildButton.height - 6

        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Backspace },
                { Key.RightAlt, Key.RightShift, Key.Backspace }
            },
            onPress = function()
                self.get.backButton.func:forcePress()
            end
        })

        local platformMenu = self.get.content:createChild("DropDownMenu", {
            x = title.x,
            y = title.y + title.height + 10,
            width = self.get.targetWidth - title.x*2
        })
        self.get.platformMenu = platformMenu
        self.func:loadPlatforms()
    end,

    loadPlatforms = function(self)
        local settings = self.world.func:getSettings()

        local platforms = {}

        if Game.platform == "windows" then
            table.insert(platforms, "label_platform_windows")
            table.insert(platforms, "label_platform_web")
        elseif Game.platform == "linux" then
            table.insert(platforms, "label_platform_linux")
        elseif Game.platform == "mac" then
            table.insert(platforms, "label_platform_mac")
        end
        
        self.get.platformMenu.func:createOptions(platforms)
        
        if settings and settings.lastBuildPlatform then
            self.get.platformMenu.func:select(settings.lastBuildPlatform)
        else
            self.get.platformMenu.func:select(platforms[1])
        end
    end,

    continueBuilding = function(self)
        self.func:closeWindow(function(win)
            local settings = self.world.func:getSettings()
            local newWindow

            local platform = self.get.platformMenu.get.selected

            if platform == "label_platform_windows" then
                if (not settings.vsBuildToolsInstalled) and (not System:VSBuildToolsInstalled()) then
                    newWindow = self.world.get.windows:createChild("VSBuildToolsInstaller", {
                        projectPath = self.get.projectPath
                    })
                    newWindow.func:openWindow()
                else
                    if not settings.vsBuildToolsInstalled then
                        settings.vsBuildToolsInstalled = true
                    end
                    newWindow = self.world.get.windows:createChild("WindowsBuildOptions", {
                        projectPath = self.get.projectPath
                    })
                    newWindow.func:openWindow()
                end
            elseif platform == "label_platform_web" then
                newWindow = self.world.get.windows:createChild("WebBuildOptions", {
                    projectPath = self.get.projectPath
                })
                newWindow.func:openWindow()
            end

            settings.lastBuildPlatform = platform
            self.world.func:saveSettings()

            if newWindow then
                newWindow.func:openWindow()
            end

            self:destroy()
        end)
    end
})