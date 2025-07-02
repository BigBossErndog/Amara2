return Nodes:define("BuildOptions", "UIWindow", {
    width = 256,
    height = 106,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))

        self.props.title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_buildOptions"),
            color = Colors.White,
            origin = 0,
            input = true
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

        self.props.iconBacker = self.props.content:createChild("FillRect", {
            x = 8, y = 28,
            width = 32,
            height = 32,
            color = Colors.Black,
            origin = 0
        })

        self.props.iconPreview = self.props.content:createChild("Sprite", {
            x = self.props.iconBacker.x,
            y = self.props.iconBacker.y,
            visible = false,
            origin = 0
        })

        local exeIconTitle = self.props.content:createChild("Text", {
            x = self.props.iconBacker.x + self.props.iconBacker.width + 8,
            y = self.props.iconBacker.y,
            text = Localize:get("title_executableIcon"),
            font = "defaultFont",
            color = Colors.White,
            origin = 0
        })

        self.props.iconField = self.props.content:createChild("TextField", {
            x = self.props.iconBacker.x + self.props.iconBacker.width + 6,
            y = self.props.iconBacker.y + 14,
            width = self.props.targetWidth - exeIconTitle.x - 8 - 18,
            inputEnabled = false,
            defaultText = Localize:get("label_selectIcon")
        })

        self.props.browseButton = self.props.content:createChild("UIButton", {
            id = "browseButton",
            toolTip = "toolTip_browseFile",
            x = self.props.iconField.x + self.props.iconField.width + 4,
            y = self.props.iconField.y,
            icon = 6,
            onPress = function()
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    self.props.iconPath = nil

                    local path = System:browseFile(self.props.projectPath)

                    self.world:showWindow()
                    
                    if string.len(path) == 0 then
                        return
                    end

                    self.props.iconField.func:setText("")
                    if self.func:checkIcon(path) then
                        self.props.iconPath = path
                        self.props.iconField.func:setText(self.func:truncatePath(path))
                    end
                end)
            end
        })

        self.props.errorMessage = self.props.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 64
        })

        if projectData["exe-icon"] then
            local path = projectData["exe-icon"]
            if self.func:checkIcon(path) then
                self.props.iconPath = path
                self.props.iconField.func:setText(self.func:truncatePath(path))
            else
                projectData["exe-icon"] = nil
                System:writeFile(System:join(self.props.projectPath, "project.json"), projectData)
            end
            self.props.errorMessage.visible = false
        end

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

    checkIcon = function(self, path)
        self.props.iconPreview.visible = false
        self.props.errorMessage.visible = false
        if not System:exists(path) then
            return false
        end
        local validFileExt = { ".png", ".jpg", ".jpeg", ".bmp" }
        local valid = false
        for i = 1, #validFileExt do
            if string.sub(path, -#validFileExt[i]) == validFileExt[i] then
                valid = true
                break
            end
        end
        if not valid then
            self.props.errorMessage.text = Localize:get("error_invalidIcon")
            self.props.errorMessage.visible = true
            return false
        end
        self.load:image("iconPreview", path)
        self.props.iconPreview.texture = "iconPreview"
        if self.props.iconPreview.width == 256 and self.props.iconPreview.height == 256 then
            self.props.iconPreview.visible = true
            self.props.iconPreview.rect = { 
                self.props.iconPreview.x,
                self.props.iconPreview.y,
                32, 32
            }
            return true
        end
        self.props.errorMessage.text = Localize:get("error_invalidIconSize")
        self.props.errorMessage.visible = true
        return false
    end,

    startBuilding = function(self)
        self.func:closeWindow(function()
            local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))
            projectData["exe-icon"] = self.props.iconPath
            System:writeFile(System:join(self.props.projectPath, "project.json"), projectData)

            self.world.props.windows:createChild("WindowsBuildNode", {
                projectPath = self.props.projectPath,
                iconPath = self.props.iconPath
            })
        end)
    end,

    truncatePath = function(self, _path)
        local txt = self.props.iconField.props.txt
        local str = _path
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.props.iconField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end
})