return NodeFactory:create("NewProjectWindow", "UIWindow", {
    width = 256,
    height = 120,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.props.folderPath = System:getRelativePath("projects")

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
                if string.len(self.props.nameField.props.finalText) ~= 0 then
                    self.props.errorMessage.visible = false
                end
            end
        })

        self.props.folderField = self.props.content:createChild("TextField", {
            x = 8, y = 28 + 22,
            width = self.props.targetWidth - 34,
            inputEnabled = false,
            defaultText = "Project Folder Path"
        })

        self.props.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseFolder",
            x = self.props.folderField.x + self.props.folderField.width + 4,
            y = self.props.folderField.y,
            icon = 5,
            onPress = function()
                self.world.screenMode = ScreenMode.BorderlessWindowed
                self.world:minimizeWindow()

                self:wait(0.2):next(function()
                    local path = System:browseFolder(self.props.folderPath)

                    self.world:restoreWindow()
                    self.world.screenMode = ScreenMode.BorderlessFullscreen

                    if string.len(path) == 0 then
                        return
                    end

                    self.props.folderPath = path

                    local txt = self.props.nameField.props.finalText
                    self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
                
                    if string.len(self.props.nameField.props.finalText) ~= 0 then
                        self.props.errorMessage.visible = false
                    end
                end)
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

        self.props.errorMessage = self.props.content:createChild("Text", {
            text = Localize:get("error_emptyProjectName"),
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 72
        })

        local createButton = self.props.content:createChild("UIButton", {
            id = "createProjectButton",
            text = "label_createProject",
            onPress = function()
                if string.len(self.props.nameField.props.finalText) == 0 then
                    self.props.errorMessage.visible = true
                else
                    self.props.errorMessage.visible = false
                end
            end
        })
        createButton.x = self.props.targetWidth - createButton.width - 8
        createButton.y = 96

        local txt = self.props.nameField.props.finalText
        self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
    end,

    makePath = function(self, defPath, target)
        local txt = self.props.folderField.props.txt
        local str
        if Game.platform == "windows" then
            str = string.concat(defPath, "\\", target)
        else 
            str = string.concat(defPath, "/", target)
        end

        local edited = false
        txt.text = str
        while txt.width > self.props.folderField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end
})