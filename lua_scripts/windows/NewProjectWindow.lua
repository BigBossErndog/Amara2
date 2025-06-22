return NodeFactory:create("NewProjectWindow", "UIWindow", {
    width = 256,
    height = 120,

    props = {
        folderPath = "C:/Ernest/Projects/"
    },

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_newProject"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        self.props.nameField = self.props.content:createChild("TextField", {
            x = 8, y = 28,
            width = self.props.targetWidth - 16,
            defaultText = "Project Name",
            onChange = function(textField, txt)
                self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
            end
        })

        self.props.folderField = self.props.content:createChild("TextField", {
            x = 8, y = 28 + 22,
            width = self.props.targetWidth - 36,
            inputEnabled = false,
            defaultText = "Project Folder Path"
        })

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_openFolder",
            x = self.props.folderField.x + self.props.folderField.width + 4,
            y = self.props.folderField.y,
            icon = 5,
            onPress = function()
                
            end
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
    end,

    makePath = function(self, defPath, target)
        local txt = self.props.folderField.props.txt
        local str = string.concat(defPath, target)
        local edited = false
        txt.text = str
        while txt.width > self.props.folderField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end
})