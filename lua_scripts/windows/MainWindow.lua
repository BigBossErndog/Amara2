return NodeFactory:create("MainWindow", "UIWindow", {
    width = 256,
    height = 120,
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 8, y = 8,
            font = "defaultFont",
            text = Localize:get("recentProjects_title"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        self.props.content:createChild("FillRect", {
            x = 6, y = 24,
            width = self.props.targetWidth - 12,
            height = 64,
            color = "#111d27",
            origin = 0
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 18

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 0,
            onPress = function()
                self.func:exitWindow()
                self.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 3,
            onPress = function(self)
                self.world:minimizeWindow()
                self.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_newProject",
            x = buttonPos,
            y = 4,
            icon = 1,
            onPress = function()
                self.func:closeWindow(function()
                    self.props.enabled = false
                    
                    local newWindow = self.parent:createChild("NewProjectWindow")
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
    end,
    exitWindow = function(self)
        self.func:closeWindow(function(self)
            self.world:destroy()
        end)
    end
})