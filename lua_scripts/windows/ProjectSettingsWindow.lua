Nodes:define("ProjectSettingsWindow", "UIWindow", {
    width = 256,
    height = 140,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.oldProjectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
        local projectData = System:readJSON(System:join(self.get.oldProjectPath, "project.json"))

        self.get.folderPath = System:getDirectoryOf(self.get.oldProjectPath)
        self.get.projectPath = ""

        local title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_projectSettings"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        self.get.content:createChild("Text", {
            x = 10, y = 28,
            origin = 0,
            font = "defaultFont",
            text = Localize:get("label_projectName")
        })

        self.get.nameField = self.get.content:createChild("TextField", {
            x = 8, y = 42,
            width = self.get.targetWidth - 16,
            defaultText = Localize:get("label_projectName"),
            onChange = function(textField, txt)
                self.get.folderField.func:setText(self.func:makePath(self.get.folderPath, txt))
            end,
            onEnter = function()
                if not self.func:checkPath() then
                    self.get.nameField.func:focusField()
                else
                    self.func:createProject()
                end
            end
        })
        self.get.nameField.func:setText(projectData["project-name"])

        self.get.content:createChild("Text", {
            x = 10, y = 64,
            origin = 0,
            font = "defaultFont",
            text = Localize:get("label_projectLocation")
        })

        self.get.folderField = self.get.content:createChild("TextField", {
            x = 8, y = 78,
            width = self.get.targetWidth - 34,
            inputEnabled = false
        })

        self.get.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseDirectory",
            x = self.get.folderField.x + self.get.folderField.width + 4,
            y = self.get.folderField.y,
            icon = 6,
            onPress = function()
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    local path = System:browseDirectory(self.get.folderPath)
                    
                    self.world:showWindow()

                    if string.len(path) == 0 then
                        return
                    end

                    self.get.folderPath = path

                    local txt = self.get.nameField.get.finalText
                    self.get.folderField.func:setText(self.func:makePath(self.get.folderPath, txt))
                end)
            end
        })

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
                self.func:closeWindow(function()
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.oldProjectPath
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

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "minimizeButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function(button)
                self.world:minimizeWindow()
                button.get.enabled = false
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Minus },
                { Key.RightAlt, Key.RightShift, Key.Minus }
            }
        })

        self.get.errorMessage = self.get.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 98
        })

        local saveButton = self.get.content:createChild("UIButton", {
            id = "saveButton",
            text = "label_saveSettings",
            onPress = function(btn)
                if self.func:checkPath() then
                    btn.get.enabled = false
                    self.func:createProject()
                end
            end
        })
        saveButton.x = self.get.targetWidth - saveButton.width - 8
        saveButton.y = 116

        local txt = self.get.nameField.get.finalText
        self.get.folderField.func:setText(self.func:makePath(self.get.folderPath, txt))
    end,

    checkPath = function(self)
        if string.len(self.get.nameField.get.finalText) == 0 then
            self.get.errorMessage.text = Localize:get("error_emptyProjectName")
            self.get.errorMessage.visible = true
        elseif System:equivalent(self.get.projectPath, self.get.oldProjectPath) then
            self.get.errorMessage.visible = false
            return false
        elseif System:isDirectory(self.get.projectPath) then
            self.get.errorMessage.text = Localize:get("error_directoryAlreadyExists")
            self.get.errorMessage.visible = true
        elseif System:exists(self.get.projectPath) then
            self.get.errorMessage.text = Localize:get("error_pathToFile")
            self.get.errorMessage.visible = true
        else
            self.get.errorMessage.visible = false
            return true
        end
        return false
    end,

    makePath = function(self, defPath, target)
        local txt = self.get.folderField.get.txt
        local str
        if Game.platform == "windows" then
            str = string.concat(defPath, "\\", target)
        else 
            str = string.concat(defPath, "/", target)
        end
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.get.folderField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        self.get.projectPath = path
        return txt.text
    end,

    createProject = function(self)
        local projectName = self.get.nameField.get.finalText

        local oldProjectDirectory = self.get.oldProjectPath
        local newProjectDirectory = self.get.projectPath
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