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
            onPress = function(btn)
                if self.func:checkPath() then
                    btn.props.enabled = false
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
            self.props.errorMessage.visible = false
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
        local projectName = self.props.nameField.props.finalText

        local oldProjectDirectory = self.props.oldProjectPath
        local newProjectDirectory = self.props.projectPath
        local parent = self.parent
        local returnWindow = function()
            local newWindow = parent:createChild("ProjectWindow", {
                projectPath = newProjectDirectory
            })
            newWindow.func:openDefault()
            newWindow.func:openWindow()
        end
        
        self.func:closeWindow(function()
            self.func:closeWindow(function()
                if oldProjectDirectory ~= newProjectDirectory then
                    local newProcess = parent:createChild("ProcessNode", {
                        arguments = {
                            Game.executable,
                            "-script", System:getScriptPath("utility/MoveProject.lua"),
                            "-oldProjectDirectory",
                            oldProjectDirectory,
                            "-newProjectDirectory",
                            newProjectDirectory,
                            "-projectName",
                            projectName
                        },
                        onExit = function(process, exitCode)
                            if exitCode == 0 then
                                returnWindow()
                            else
                                if System:exists(newProjectDirectory) then
                                    System:remove(newProjectDirectory)
                                end
                                newProjectDirectory = oldProjectDirectory
                                local newTerminal = parent:createChild("TerminalWindow", {
                                    allowMinimize = true,
                                    disableSavePosition = true,
                                    onExit = function(self)
                                        returnWindow()
                                    end
                                })
                                newTerminal.func:handleMessage(Localize:get("error_failedToMoveProject"))
                                newTerminal.func:openWindow()
                            end
                        end
                    })
                else
                    onSuccess()
                    returnWindow()
                end
                
                self:destroy()
            end)
        end)
    end
})