Nodes:define("WindowsBuildOptions", "PagedWindow", {
    width = 256,
    height = 140,

    props = {
        pageCount = 4
    },

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.props.projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))

        self.classes.PagedWindow.func:onCreate()
        
        self.props.errorMessage = self.props.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 98
        })

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
        local backButton = self.props.content:createChild("UIButton", {
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
    end,

    onCreatePage = function(self, pageIndex)
        if pageIndex == 1 then
            local exeNameTitle = self.props.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("title_executableFileName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.props.exeNameField = self.props.pageContent:createChild("TextField", {
                x = 8, y = exeNameTitle.y + exeNameTitle.height + 4,
                width = self.props.targetWidth - 16,

                defaultText = Localize:get("label_enterExecutableFileName"),

                onCreate = function(self)
                    self.classes.TextField.func:onCreate(self)

                    self.props.exeTxt = self:createChild("Text", {
                        x = 8, y = 2,
                        font = "defaultFont",
                        text = ".exe",
                        color = "#515f73",
                        origin = 0,
                        visible = false,
                        props = {
                            backing = self
                        }
                    })

                    self.props.maxTextWidth = self.width - 16 - self.props.exeTxt.width
                end,

                onChange = function(textField, txt)
                    textField.props.exeTxt.visible = true
                    textField.props.exeTxt.x = textField.props.txt.x + textField.props.txt.width

                    if string.len(txt) > 0 then
                        self.props.projectData["executable-name"] = self.props.exeNameField.props.finalText
                    end
                end,

                onFocus = function(self)
                    self.props.exeTxt.visible = true
                    self.props.exeTxt.x = self.props.txt.x + self.props.txt.width
                end,

                onUnfocus = function(self)
                    if string.len(self.props.finalText) == 0 then
                        self.props.exeTxt.visible = false
                    end
                end,

                onUpdate = function(self, deltaTime)
                    self.classes.TextField.func:onUpdate(self, deltaTime)
                end
            })

            if self.props.projectData["executable-name"] then
                self.props.exeNameField.func:setText(self.props.projectData["executable-name"])
                self.props.exeNameField.func:onChange(self.props.projectData["executable-name"])
            end

            self.props.iconBacker = self.props.pageContent:createChild("FillRect", {
                x = 8, y = 64,
                width = 32,
                height = 32,
                color = Colors.Black,
                origin = 0
            })

            self.props.iconPreview = self.props.pageContent:createChild("Sprite", {
                x = self.props.iconBacker.x,
                y = self.props.iconBacker.y,
                visible = false,
                origin = 0
            })

            local exeIconTitle = self.props.pageContent:createChild("Text", {
                x = self.props.iconBacker.x + self.props.iconBacker.width + 8,
                y = self.props.iconBacker.y,
                text = Localize:get("title_executableIcon"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.props.iconField = self.props.pageContent:createChild("TextField", {
                x = self.props.iconBacker.x + self.props.iconBacker.width + 6,
                y = self.props.iconBacker.y + 14,
                width = self.props.targetWidth - exeIconTitle.x - 8 - 18,
                inputEnabled = false,
                defaultText = Localize:get("label_selectIcon")
            })

            self.props.browseButton = self.props.pageContent:createChild("UIButton", {
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

            if self.props.projectData["exe-icon"] then
                local path = self.props.projectData["exe-icon"]
                if self.func:checkIcon(path) then
                    self.props.iconPath = path
                    self.props.iconField.func:setText(self.func:truncatePath(path))
                else
                    self.props.projectData["exe-icon"] = nil
                    System:writeFile(System:join(self.props.projectPath, "project.json"), self.props.projectData)
                end
                self.props.errorMessage.visible = false
            end
        elseif pageIndex == 2 then
            local desc = self.props.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("label_compilationDesc"),
                font = "defaultFont",
                color = Colors.Yellow,
                wrapWidth = self.props.targetWidth - 20,
                wrapMode = WrapMode.ByWord,
                origin = 0
            })

            local backer = self.props.pageContent:createChild("FillRect", {
                x = 10, y = desc.y + desc.height + 8,
                width = self.props.targetWidth - 20,
                height = 18,
                color = "#111d27",
                origin = 0,
                input = {
                    active = true,
                    cursor = Cursor.Pointer,
                    onPointerDown = function()
                        self.props.projectData["compile-code"] = not self.props.projectData["compile-code"]
                        self.props.tickBox.frame = self.props.projectData["compile-code"] and 2 or 1
                    end
                }
            })
            
            local compileTxt = self.props.pageContent:createChild("Text", {
                x = backer.x + 8, y = backer.y + 2,
                text = Localize:get("label_compileCode"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.props.tickBox = self.props.pageContent:createChild("Sprite", {
                origin = { 1, 0 },
                x = compileTxt.x + compileTxt.width + 12,
                y = compileTxt.y + 2,
                frame = self.props.projectData["compile-code"] and 2 or 1,
                texture = "tickBox"
            })
        elseif pageIndex == 3 then
            local desc = self.props.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("label_encryptionDesc"),
                font = "defaultFont",
                color = Colors.Yellow,
                wrapWidth = self.props.targetWidth - 20,
                wrapMode = WrapMode.ByWord,
                origin = 0
            })

            local encryptionButton = self.props.pageContent:createChild("UIButton", {
                id = "encryptionButton",
                text = "label_openEncryptionOptions",
                onPress = function()
                    self.func:closeWindow(function(win)
                        local newWindow = self.world.props.windows:createChild("EncryptionOptions", {
                            projectPath = self.props.projectPath,
                            projectData = self.props.projectData,
                            returnWindow = self,
                            width = self.props.targetWidth,
                            height = self.props.targetHeight,
                            x = self.x,
                            y = self.y
                        })
                        newWindow.func:openWindow()

                        win.visible = false
                    end)
                end
            })
            encryptionButton.x = self.props.targetWidth/2 - encryptionButton.width/2
            encryptionButton.y = desc.y + desc.height + 6

            self.props.tickBox = self.props.pageContent:createChild("Sprite", {
                origin = 0,
                x = encryptionButton.x + encryptionButton.width + 5,
                y = encryptionButton.y + 4,
                frame = self.props.projectData["compile-code"] and 2 or 1,
                texture = "tickBox"
            })

            self.func:checkEncryption()
        elseif pageIndex == self.props.pageCount then
            local backer = self.props.pageContent:createChild("FillRect", {
                x = 6, y = 24,
                width = self.props.targetWidth - 12,
                height = 86,
                color = "#111d27",
                origin = 0
            })
            local buildButton = self.props.pageContent:createChild("UIButton", {
                id = "buildProjectButton",
                text = "label_buildProject",
                onPress = function()
                    self.func:startBuilding()
                end
            })
            buildButton.x = self.props.targetWidth/2 - buildButton.width/2
            buildButton.y = self.props.targetHeight/2 - buildButton.height/2 - 2
        end
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
            self.props.projectData["exe-icon"] = self.props.iconPath
            return true
        end
        self.props.errorMessage.text = Localize:get("error_invalidIconSize")
        self.props.errorMessage.visible = true
        return false
    end,

    checkEncryption = function(self)
        self.props.tickBox.frame = self.props.projectData["encryption"] and 2 or 1
    end,

    startBuilding = function(self)
        self.func:closeWindow(function(win)
            System:writeFile(System:join(self.props.projectPath, "project.json"), self.props.projectData)
            
            self.world.props.windows:createChild("WindowsBuildNode", {
                projectPath = self.props.projectPath,
                iconPath = self.props.iconPath
            })

            win:destroy()
        end)
    end,

    setPage = function(self, pageIndex)
        if self.props.pageIndex == 1 then
            if self.props.exeNameField and self.props.exeNameField.props.finalText == "" then
                self.props.errorMessage.text = Localize:get("error_emptyExecutableFileName")
                self.props.errorMessage.visible = true
                return false
            end
        end

        if self.props.errorMessage then
            self.props.errorMessage.visible = false
        end
        return self.classes.PagedWindow.func:setPage(pageIndex)
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