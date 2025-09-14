Nodes:define("ProjectSettingsWindow", "UIWindow", {
    width = 256,
    height = 140,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.oldProjectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
        local projectData = System:readJSON(System:join(self.props.oldProjectPath, "project.json"))

        self.props.folderPath = System:getDirectoryOf(self.props.oldProjectPath)
        self.props.projectPath = ""

        local title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_projectSettings"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        self.props.content:createChild("Text", {
            x = 10, y = 28,
            origin = 0,
            font = "defaultFont",
            text = Localize:get("label_projectName")
        })

        self.props.nameField = self.props.content:createChild("TextField", {
            x = 8, y = 42,
            width = self.props.targetWidth - 16,
            defaultText = Localize:get("label_projectName"),
            onChange = function(textField, txt)
                self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
            end,
            onEnter = function()
                if not self.func:checkPath() then
                    self.props.nameField.func:focusField()
                else
                    self.func:createProject()
                end
            end
        })
        self.props.nameField.func:setText(projectData["project-name"])

        self.props.content:createChild("Text", {
            x = 10, y = 64,
            origin = 0,
            font = "defaultFont",
            text = Localize:get("label_projectLocation")
        })

        self.props.folderField = self.props.content:createChild("TextField", {
            x = 8, y = 78,
            width = self.props.targetWidth - 34,
            inputEnabled = false
        })

        self.props.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseDirectory",
            x = self.props.folderField.x + self.props.folderField.width + 4,
            y = self.props.folderField.y,
            icon = 6,
            onPress = function()
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    local path = System:browseDirectory(self.props.folderPath)
                    
                    self.world:showWindow()

                    if string.len(path) == 0 then
                        return
                    end

                    self.props.folderPath = path

                    local txt = self.props.nameField.props.finalText
                    self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
                end)
            end
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 1,
            onPress = function(button)
                self.world.props.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                button.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function(button)
                self.world:minimizeWindow()
                button.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.props.enabled = false
                self.func:closeWindow(function()
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.props.oldProjectPath
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        self.props.errorMessage = self.props.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 98
        })

        local saveButton = self.props.content:createChild("UIButton", {
            id = "saveButton",
            text = "label_saveSettings",
            onPress = function()
                if self.func:checkPath() then
                    self.func:createProject()
                end
            end
        })
        saveButton.x = self.props.targetWidth - saveButton.width - 8
        saveButton.y = 116

        local txt = self.props.nameField.props.finalText
        self.props.folderField.func:setText(self.func:makePath(self.props.folderPath, txt))
    end,

    checkPath = function(self)
        if string.len(self.props.nameField.props.finalText) == 0 then
            self.props.errorMessage.text = Localize:get("error_emptyProjectName")
            self.props.errorMessage.visible = true
        elseif System:equivalent(self.props.projectPath, self.props.oldProjectPath) then
            return false
        elseif System:isDirectory(self.props.projectPath) then
            self.props.errorMessage.text = Localize:get("error_directoryAlreadyExists")
            self.props.errorMessage.visible = true
        elseif System:exists(self.props.projectPath) then
            self.props.errorMessage.text = Localize:get("error_pathToFile")
            self.props.errorMessage.visible = true
        else
            self.props.errorMessage.visible = false
            return true
        end
        return false
    end,

    makePath = function(self, defPath, target)
        local txt = self.props.folderField.props.txt
        local str
        if Game.platform == "windows" then
            str = string.concat(defPath, "\\", target)
        else 
            str = string.concat(defPath, "/", target)
        end
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.props.folderField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        self.props.projectPath = path
        return txt.text
    end,

    createProject = function(self)
        if self.props.oldProjectPath ~= self.props.projectPath then
            System:createDirectory(self.props.projectPath)
            System:copy(
                self.props.oldProjectPath,
                self.props.projectPath
            )
        end

        local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))
        projectData["project-name"] = self.props.nameField.props.finalText
        projectData["executable-name"] = self.props.nameField.props.finalText

        projectData.uninitiated = true

        System:writeFile(System:join(self.props.projectPath, "project.json"), projectData)

        self.func:closeWindow(function()
            self.func:closeWindow(function()
                self.props.enabled = false

                local newWindow = self.parent:createChild("ProjectWindow", {
                    projectPath = self.props.projectPath
                })
                newWindow.func:openDefault()
                newWindow.func:openWindow()
                
                self:destroy()
            end)
        end)
    end
})