Nodes:define("WebBuildOptions", "UIWindow", {
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
            text = Localize:get("title_buildOptions"),
            color = Colors.White,
            origin = 0
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
            local buildNode = self.world.props.windows:createChild("WebBuildNode", {
                projectPath = self.props.projectPath
            })
            self:destroy()
        end)
    end
})