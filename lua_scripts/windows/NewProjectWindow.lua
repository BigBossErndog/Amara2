return NodeFactory:create("NewProjectWindow", "UIWindow", {
    width = 256,
    height = 200,
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 8, y = 8,
            font = "defaultFont",
            text = Localize:get("newProject_title"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 18

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function()
                self.func:closeWindow(function()
                    self.props.enabled = false
                    
                    local newWindow = self.parent:createChild("MainWindow")
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
    end
})