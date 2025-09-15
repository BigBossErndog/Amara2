Nodes:define("WebBuildOptions", "UIWindow", {
    width = 256,
    height = 140,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.props.projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))

        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_buildOptions"),
            color = Colors.White,
            origin = 0
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        self.props.backButton = self.props.content:createChild("UIButton", {
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
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Backspace },
                { Key.RightAlt, Key.RightShift, Key.Backspace }
            },
            onPress = function()
                self.props.backButton.func:forcePress()
            end
        })

        local txt = self.props.content:createChild("Text", {
            x = 10, y = 24,
            origin = 0,
            text = Localize:get("label_includeFoldersDesc"),
            font = "defaultFont",
            color = Colors.White,
        })
        self.props.includeFolders = self.props.content:createChild("IncludeFolders", {
            projectPath = self.props.projectPath,
            projectData = self.props.projectData,
            x = 10, y = 40,
            width = self.props.targetWidth - 20,
            height= self.props.targetHeight - 40 - 28
        })

        local buildButton = self.props.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_buildProject",
            onPress = function()
                self.func:startBuilding()
            end
        })
        buildButton.x = self.props.targetWidth - buildButton.width - 8
        buildButton.y = self.props.targetHeight - buildButton.height - 6
    end,

    startBuilding = function(self)
        self.func:closeWindow(function(self)
            self.props.includeFolders.func:confirmOptions()
            System:writeFile(System:join(self.props.projectPath, "project.json"), self.props.projectData)
            
            local buildNode = self.world.props.windows:createChild("WebBuildNode", {
                projectPath = self.props.projectPath
            })
            self:destroy()
        end)
    end
})