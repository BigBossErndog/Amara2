Nodes:define("NewProjectWindow", "UIWindow", {
    width = 256,
    height = 120,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local settings = self.world.func:getSettings()

        self.get.folderPath = System:getRelativePath("projects")
        self.get.projectPath = ""

        if settings.projects and #settings.projects > 0 then
            if System:exists(System:getDirectoryOf(settings.projects[1])) then
                self.get.folderPath = System:getDirectoryOf(settings.projects[1])
            end
        end

        local title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_newProject"),
            color = "#f0f6ff",
            input = true
        })

        self.get.nameField = self.get.content:createChild("TextField", {
            x = 8, y = 28,
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

        self.get.folderField = self.get.content:createChild("TextField", {
            x = 8, y = 28 + 22,
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

        self.get.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 1,
            onPress = function()
                self.world.get.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                self.get.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function(self)
                self.world:minimizeWindow()
                self.get.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function()
                self.func:closeWindow(function()
                    self.get.enabled = false
                    
                    local newWindow = self.parent:createChild("MainWindow", {
                        x = self.x, y = self.y
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

        self.get.errorMessage = self.get.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 72
        })

        local createButton = self.get.content:createChild("UIButton", {
            id = "createProjectButton",
            text = "label_createProject",
            onPress = function()
                if self.func:checkPath() then
                    self.func:createProject()
                end
            end
        })
        createButton.x = self.get.targetWidth - createButton.width - 8
        createButton.y = 96

        local txt = self.get.nameField.get.finalText
        self.get.folderField.func:setText(self.func:makePath(self.get.folderPath, txt))
    end,

    checkPath = function(self)
        if string.len(self.get.nameField.get.finalText) == 0 then
            self.get.errorMessage.text = Localize:get("error_emptyProjectName")
            self.get.errorMessage.visible = true
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
        System:createDirectory(self.get.projectPath)
        System:copy(
            System:join(System:getBasePath(), "files", "defaultTemplate"),
            self.get.projectPath
        )

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        projectData["project-name"] = self.get.nameField.get.finalText
        projectData["executable-name"] = self.get.nameField.get.finalText

        projectData.uninitiated = true

        System:writeFile(System:join(self.get.projectPath, "project.json"), projectData)
        
        local indexFile = System:readFile(System:join(self.get.projectPath, "lua_scripts", "index.lua"))
        local fixedIndexFile = string.gsub(indexFile, "${Window_Title}", self.get.nameField.get.finalText)
        System:writeFile(System:join(self.get.projectPath, "lua_scripts", "index.lua"), fixedIndexFile)

        System:createDirectory(System:join(self.get.projectPath, "files"))

        self.func:closeWindow(function()
            self.func:closeWindow(function()
                self.get.enabled = false

                local newWindow = self.parent:createChild("ProjectWindow", {
                    projectPath = self.get.projectPath
                })
                newWindow.func:openDefault()
                newWindow.func:openWindow()
                
                self:destroy()
            end)
        end)
    end
})