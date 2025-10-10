Nodes:define("WindowsBuildOptions", "PagedWindow", {
    width = 256,
    height = 140,

    props = {
        pageCount = 5
    },

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
    end,

    onCreate = function(self)
        self.get.projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))

        self.classes.PagedWindow.func:onCreate()
        
        self.get.errorMessage = self.get.content:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            color = Colors.Red,
            visible = false,
            x = 10, y = 98
        })

        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_buildOptions"),
            color = Colors.White,
            origin = 0,
            input = true
        })
        
        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        local backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
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
                backButton.func:forcePress()
            end
        })
    end,

    onCreatePage = function(self, pageIndex)
        if pageIndex == 1 then
            local exeNameTitle = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("title_executableFileName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.exeNameField = self.get.pageContent:createChild("TextField", {
                x = 8, y = exeNameTitle.y + exeNameTitle.height + 4,
                width = self.get.targetWidth - 16,

                defaultText = Localize:get("label_enterExecutableFileName"),

                onCreate = function(self)
                    self.classes.TextField.func:onCreate(self)

                    self.get.exeTxt = self:createChild("Text", {
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

                    self.get.maxTextWidth = self.width - 16 - self.get.exeTxt.width
                end,

                onChange = function(textField, txt)
                    textField.get.exeTxt.visible = true
                    textField.get.exeTxt.x = textField.get.txt.x + textField.get.txt.width

                    if string.len(txt) > 0 then
                        self.get.projectData["executable-name"] = self.get.exeNameField.get.finalText
                    end
                end,

                onFocus = function(self)
                    self.get.exeTxt.visible = true
                    self.get.exeTxt.x = self.get.txt.x + self.get.txt.width
                end,

                onUnfocus = function(self)
                    if string.len(self.get.finalText) == 0 then
                        self.get.exeTxt.visible = false
                    end
                end,

                onUpdate = function(self, deltaTime)
                    self.classes.TextField.func:onUpdate(self, deltaTime)
                end
            })

            if self.get.projectData["executable-name"] then
                self.get.exeNameField.func:setText(self.get.projectData["executable-name"])
                self.get.exeNameField.func:onChange(self.get.projectData["executable-name"])
            end

            self.get.iconBacker = self.get.pageContent:createChild("FillRect", {
                x = 8, y = 64,
                width = 32,
                height = 32,
                color = Colors.Black,
                origin = 0
            })

            self.get.iconPreview = self.get.pageContent:createChild("Sprite", {
                x = self.get.iconBacker.x,
                y = self.get.iconBacker.y,
                visible = false,
                origin = 0
            })

            local exeIconTitle = self.get.pageContent:createChild("Text", {
                x = self.get.iconBacker.x + self.get.iconBacker.width + 8,
                y = self.get.iconBacker.y,
                text = Localize:get("title_executableIcon"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.iconField = self.get.pageContent:createChild("TextField", {
                x = self.get.iconBacker.x + self.get.iconBacker.width + 6,
                y = self.get.iconBacker.y + 14,
                width = self.get.targetWidth - exeIconTitle.x - 8 - 18,
                inputEnabled = false,
                defaultText = Localize:get("label_selectIcon")
            })

            self.get.browseButton = self.get.pageContent:createChild("UIButton", {
                id = "browseButton",
                toolTip = "toolTip_browseFile",
                x = self.get.iconField.x + self.get.iconField.width + 4,
                y = self.get.iconField.y,
                icon = 6,
                onPress = function()
                    self.world:hideWindow()

                    self:wait(0.2):next(function()
                        self.get.iconPath = nil

                        local path = System:browseFile(self.get.projectPath)

                        self.world:showWindow()
                        
                        if string.len(path) == 0 then
                            return
                        end

                        self.get.iconField.func:setText("")
                        if self.func:checkIcon(path) then
                            self.get.iconPath = path
                            self.get.iconField.func:setText(self.func:truncatePath(path))
                        end
                    end)
                end
            })

            if self.get.projectData["exe-icon"] then
                local path = self.get.projectData["exe-icon"]
                if self.func:checkIcon(path) then
                    self.get.iconPath = path
                    self.get.iconField.func:setText(self.func:truncatePath(path))
                else
                    self.get.projectData["exe-icon"] = nil
                    System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
                end
                self.get.errorMessage.visible = false
            end
        elseif pageIndex == 2 then
            local desc = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("label_compilationDesc"),
                font = "defaultFont",
                color = Colors.Yellow,
                wrapWidth = self.get.targetWidth - 20,
                wrapMode = WrapMode.ByWord,
                origin = 0
            })

            local backer = self.get.pageContent:createChild("FillRect", {
                x = 10, y = desc.y + desc.height + 8,
                width = self.get.targetWidth - 20,
                height = 18,
                color = "#111d27",
                origin = 0,
                input = {
                    active = true,
                    cursor = Cursor.Pointer,
                    onPointerDown = function()
                        self.get.projectData["compile-code"] = not self.get.projectData["compile-code"]
                        self.get.tickBox.frame = self.get.projectData["compile-code"] and 2 or 1
                    end
                }
            })
            
            local compileTxt = self.get.pageContent:createChild("Text", {
                x = backer.x + 8, y = backer.y + 2,
                text = Localize:get("label_compileCode"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.tickBox = self.get.pageContent:createChild("Sprite", {
                origin = 0,
                x = compileTxt.x + compileTxt.width + 5,
                y = compileTxt.y + 2,
                frame = self.get.projectData["compile-code"] and 2 or 1,
                texture = "tickBox"
            })
        elseif pageIndex == 3 then
            local desc = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("label_encryptionDesc"),
                font = "defaultFont",
                color = Colors.Yellow,
                wrapWidth = self.get.targetWidth - 20,
                wrapMode = WrapMode.ByWord,
                origin = 0
            })

            local encryptionButton = self.get.pageContent:createChild("UIButton", {
                id = "encryptionButton",
                text = "label_openEncryptionOptions",
                onPress = function()
                    self.func:closeWindow(function(win)
                        local newWindow = self.world.get.windows:createChild("EncryptionOptions", {
                            projectPath = self.get.projectPath,
                            projectData = self.get.projectData,
                            returnWindow = self,
                            x = self.x,
                            y = self.y
                        })
                        newWindow.func:openWindow()

                        win.visible = false
                    end)
                end
            })
            encryptionButton.x = self.get.targetWidth/2 - encryptionButton.width/2
            encryptionButton.y = desc.y + desc.height + 5

            self.get.tickBox = self.get.pageContent:createChild("Sprite", {
                origin = 0,
                x = encryptionButton.x + encryptionButton.width + 5,
                y = encryptionButton.y + 4,
                frame = self.get.projectData["compile-code"] and 2 or 1,
                texture = "tickBox"
            })

            self.func:checkEncryption()
        elseif pageIndex == 4 then
            local txt = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                origin = 0,
                text = Localize:get("label_includeFoldersDesc"),
                font = "defaultFont",
                color = Colors.White,
            })
            self.get.includeFolders = self.get.pageContent:createChild("IncludeFolders", {
                projectPath = self.get.projectPath,
                projectData = self.get.projectData,
                x = 10, y = 40,
                width = self.get.targetWidth - 20,
                height= self.get.targetHeight - 40 - 28
            })
        elseif pageIndex == self.get.pageCount then
            local backer = self.get.pageContent:createChild("FillRect", {
                x = 10, y = 26,
                width = self.get.targetWidth - 20,
                height = 86,
                color = "#111d27",
                origin = 0,
                alpha = 0.75
            })
            local buildButton = self.get.pageContent:createChild("UIButton", {
                id = "buildProjectButton",
                text = "label_buildProject",
                onPress = function()
                    self.func:startBuilding()
                end
            })
            buildButton.x = self.get.targetWidth/2 - buildButton.width/2
            buildButton.y = self.get.targetHeight/2 - buildButton.height/2 + 4
        end
    end,

    checkIcon = function(self, path)
        self.get.iconPreview.visible = false
        self.get.errorMessage.visible = false
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
            self.get.errorMessage.text = Localize:get("error_invalidIcon")
            self.get.errorMessage.visible = true
            return false
        end
        self.load:image("iconPreview", path)
        self.get.iconPreview.texture = "iconPreview"
        if self.get.iconPreview.width == 256 and self.get.iconPreview.height == 256 then
            self.get.iconPreview.visible = true
            self.get.iconPreview.rect = { 
                self.get.iconPreview.x,
                self.get.iconPreview.y,
                32, 32
            }
            self.get.projectData["exe-icon"] = self.get.iconPath
            return true
        end
        self.get.errorMessage.text = Localize:get("error_invalidIconSize")
        self.get.errorMessage.visible = true
        return false
    end,

    checkEncryption = function(self)
        self.get.tickBox.frame = self.get.projectData["encryption"] and 2 or 1
    end,

    startBuilding = function(self)
        self.func:closeWindow(function(win)
            System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
            
            self.world.get.windows:createChild("WindowsBuildNode", {
                projectPath = self.get.projectPath,
                iconPath = self.get.iconPath
            })

            win:destroy()
            
        end)
    end,

    setPage = function(self, pageIndex)
        if self.get.pageIndex == 1 then
            if self.get.exeNameField and self.get.exeNameField.get.finalText == "" then
                self.get.errorMessage.text = Localize:get("error_emptyExecutableFileName")
                self.get.errorMessage.visible = true
                return false
            end
        elseif self.get.pageIndex == 4 then
            self.get.includeFolders.func:confirmOptions()
        end

        if self.get.errorMessage then
            self.get.errorMessage.visible = false
        end
        return self.classes.PagedWindow.func:setPage(pageIndex)
    end,

    truncatePath = function(self, _path)
        local txt = self.get.iconField.get.txt
        local str = _path
        local path = str

        local edited = false
        txt.text = str
        while txt.width > self.get.iconField.width - 16 do
            str = string.sub(str, 2)
            txt.text = string.concat("...", str)
        end
        return txt.text
    end
})