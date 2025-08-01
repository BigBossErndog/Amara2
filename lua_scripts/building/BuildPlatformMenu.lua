Nodes:define("BuildPlatformMenu", "UIWindow", {
    width = 256,
    height = 106,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_choosePlatform"),
            color = Colors.White,
            origin = 0
        })

        local platformMenu = self.props.content:createChild("DropDownMenu", {
            x = title.x,
            y = title.y + title.height + 6,
            width = self.props.targetWidth - title.x*2,
            onSelect = function(menu, opt)
                
            end
        })
        self.props.platformMenu = platformMenu
        self.func:loadPlatforms()
    end,

    loadPlatforms = function(self)
        local platforms = {}

        if Game.platform == "windows" then
            table.insert(platforms, "windows")
            table.insert(platform, "web")
        elseif Game.platform == "linux" then
            table.insert(platforms, "linux")
        elseif Game.platform == "mac" then
            table.insert(platforms, "mac")
        end
        
        self.props.platformMenu.func:createOptions(platforms)

        self.props.platformMenu.func:select(platforms[1])

        local buildButton = self.props.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                self.func:continueBuilding()
            end
        })
        buildButton.x = self.props.targetWidth - buildButton.width - 8
        buildButton.y = self.props.targetHeight - buildButton.height - 6

    end,

    continueBuilding = function(self)
        self.func:closeWindow(function(win)
            local newWindow

            self.world.forcedClickThrough = true

            local platform = self.props.platformMenu.selected

            if platform == "windows" then
                if not System:exists(System:getRelativePath("build_modules/amara2_windows_build_module/clang-llvm/bin/clang.exe")) then
                    newWindow = self.world.props.windows:createChild("WindowsBuildInstaller", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                elseif not System:VSBuildToolsInstalled() then
                    newWindow = self.world.props.windows:createChild("VSBuildToolsInstaller", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                else
                    newWindow = self.world.props.windows:createChild("WindowsBuildOptions", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                end
            end

            self.world.forcedClickThrough = false

            if newWindow then
                newWindow.func:openWindow()
            end
        end)
    end
})