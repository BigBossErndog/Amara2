Nodes:define("WebBuildOptions", "UIWindow", {
    width = 256,
    height = 140,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.exampleProject then
            self.get.exampleProject = config.exampleProject
        end
    end,

    onCreate = function(self)
        self.get.projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))

        self.super.UIWindow.func:onCreate()

        local title = self.get.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_buildOptions"),
            color = Colors.White,
            origin = 0
        })

        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath,
                        exampleProject = self.get.exampleProject
                    })
                    
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

        local txt = self.get.content:createChild("Text", {
            x = 10, y = 24,
            origin = 0,
            text = Localize:get("label_includeFoldersDesc"),
            font = "defaultFont",
            color = Colors.White,
        })
        self.get.includeFolders = self.get.content:createChild("IncludeFolders", {
            projectPath = self.get.projectPath,
            projectData = self.get.projectData,
            x = 10, y = 40,
            width = self.get.targetWidth - 20,
            height= self.get.targetHeight - 40 - 28
        })

        local buildButton = self.get.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_buildProject",
            onPress = function()
                self.func:startBuilding()
            end
        })
        buildButton.x = self.get.targetWidth - buildButton.width - 8
        buildButton.y = self.get.targetHeight - buildButton.height - 6
    end,

    startBuilding = function(self)
        self.func:closeWindow(function(self)
            self.get.includeFolders.func:confirmOptions()
            System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
            
            local buildNode = self.world.get.windows:createChild("WebBuildNode", {
                projectPath = self.get.projectPath
            })
            self:destroy()
        end)
    end
})