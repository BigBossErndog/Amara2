Nodes:define("AndroidBuildOptions", "PagedWindow", {
    id = "buildNode",

    width = 256,
    height = 160,

    props = {
        pageCount = 7
    },

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

        if not self.get.projectData.android then
            self.get.projectData.android = {}
        end
        if not self.get.projectData.android["app-name"] then
            if self.get.projectData["executable-name"] then
                self.get.projectData.android["app-name"] = self.get.projectData["executable-name"]
            elseif self.get.projectData["project-name"] then
                self.get.projectData.android["app-name"] = self.get.projectData["project-name"]
            end
        end

        self.super.PagedWindow.func:onCreate()
        
        self.get.errorMessage = self.get.content:createChild("Text", {
            font = "defaultFont",
            origin = { 0, 1 },
            color = Colors.Red,
            visible = false,
            x = 10, y = self.get.targetHeight - 28,
            wrapWidth = self.get.targetWidth - 20
        })

        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get(self.get.buildTest and "title_testBuildOptions" or "title_buildOptions"),
            color = self.get.buildTest and Colors.Green or Colors.White,
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
                backButton.func:forcePress()
            end
        })
    end,

    onCreatePage = function(self, pageIndex)
        if pageIndex == 1 then
            local appNameTitle = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("title_appName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.appNameField = self.get.pageContent:createChild("TextField", {
                x = 8, y = appNameTitle.y + appNameTitle.height + 4,
                width = self.get.targetWidth - 16,
                
                defaultText = Localize:get("label_enterAppName"),

                onCreate = function(self)
                    self.super.TextField.func:onCreate(self)

                    self.get.appTxt = self:createChild("Text", {
                        x = 8, y = 2,
                        font = "defaultFont",
                        text = ".apk",
                        color = "#515f73",
                        origin = 0,
                        visible = false,
                        props = {
                            backing = self
                        }
                    })

                    self.get.maxTextWidth = self.width - 16 - self.get.appTxt.width
                end,

                onChange = function(textField, txt)
                    textField.get.appTxt.visible = true
                    textField.get.appTxt.x = textField.get.txt.x + textField.get.txt.width + 2

                    if string.len(txt) > 0 then
                        self.get.projectData.android["app-name"] = self.get.appNameField.get.finalText
                    end
                end,

                onFocus = function(self)
                    self.get.appTxt.visible = true
                    self.get.appTxt.x = self.get.txt.x + self.get.txt.width + 2
                end,

                onUnfocus = function(self)
                    if string.len(self.get.finalText) == 0 then
                        self.get.appTxt.visible = false
                    end
                end,

                onUpdate = function(self, deltaTime)
                    self.super.TextField.func:onUpdate(self, deltaTime)
                end
            })

            if self.get.projectData.android["app-name"] then
                self.get.appNameField.func:setText(self.get.projectData.android["app-name"])
                self.get.appNameField.func:onChange(self.get.projectData.android["app-name"])
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

            local appIconTitle = self.get.pageContent:createChild("Text", {
                x = self.get.iconBacker.x + self.get.iconBacker.width + 8,
                y = self.get.iconBacker.y,
                text = Localize:get("title_appIcon"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.iconField = self.get.pageContent:createChild("TextField", {
                x = self.get.iconBacker.x + self.get.iconBacker.width + 6,
                y = self.get.iconBacker.y + 14,
                width = self.get.targetWidth - appIconTitle.x - 8 - 18,
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

            if self.get.projectData.android["app-icon"] then
                local path = self.get.projectData.android["app-icon"]
                if self.func:checkIcon(path) then
                    self.get.iconPath = path
                    self.get.iconField.func:setText(self.func:truncatePath(path))
                else
                    self.get.projectData.android["app-icon"] = nil
                    System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
                end
                if self.get.errorMessage then
                    self.get.errorMessage.visible = false
                end
            end

        elseif pageIndex == 2 then
            local packageName = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                text = Localize:get("title_packageName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            local orgTitle = self.get.pageContent:createChild("Text", {
                x = 10, y = packageName.y + packageName.height + 4,
                text = Localize:get("title_organizationName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.orgField = self.get.pageContent:createChild("TextField", {
                x = 8, y = orgTitle.y + orgTitle.height + 4,
                width = self.get.targetWidth - 16,
                
                defaultText = Localize:get("label_enterOrganizationName"),

                onChange = function(textField, txt)
                    if string.len(txt) > 0 then
                        self.get.projectData["package-org-name"] = self.get.orgField.get.finalText
                    end
                end,

                onUpdate = function(self, deltaTime)
                    self.super.TextField.func:onUpdate(self, deltaTime)
                end
            })

            local appNameTitle = self.get.pageContent:createChild("Text", {
                x = 10, y = self.get.orgField.y + self.get.orgField.height + 4,
                text = Localize:get("title_appName"),
                font = "defaultFont",
                color = Colors.White,
                origin = 0
            })

            self.get.appNameField = self.get.pageContent:createChild("TextField", {
                x = 8, y = appNameTitle.y + appNameTitle.height + 4,
                width = self.get.targetWidth - 16,
                
                defaultText = Localize:get("label_enterAppName"),

                onChange = function(textField, txt)
                    if string.len(txt) > 0 then
                        self.get.projectData["package-app-name"] = self.get.orgField.get.finalText
                    end
                end,

                onUpdate = function(self, deltaTime)
                    self.super.TextField.func:onUpdate(self, deltaTime)
                end
            })

            if self.get.projectData.android["package-org-name"] then
                self.get.orgField.func:setText(self.get.projectData.android["package-org-name"])
                self.get.orgField.func:onChange(self.get.projectData.android["package-org-name"])
            end

            if self.get.projectData.android["package-app-name"] then
                self.get.appNameField.func:setText(self.get.projectData.android["package-app-name"])
                self.get.appNameField.func:onChange(self.get.projectData.android["package-app-name"])
            end

        elseif pageIndex == 3 then
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
        elseif pageIndex == 4 then
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

        elseif pageIndex == 5 then
            local txt = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                origin = 0,
                text = Localize:get("label_includeFoldersDesc"),
                font = "defaultFont",
                color = Colors.White
            })
            self.get.includeFolders = self.get.pageContent:createChild("IncludeFolders", {
                projectPath = self.get.projectPath,
                projectData = self.get.projectData,
                x = 10, y = 40,
                width = self.get.targetWidth - 20,
                height= self.get.targetHeight - 40 - 28
            })

        elseif pageIndex == 6 then
            local txt = self.get.pageContent:createChild("Text", {
                x = 10, y = 24,
                origin = 0,
                text = Localize:get("label_pluginsDesc"),
                font = "defaultFont",
                color = Colors.White
            })
            self.get.includePlugins = self.get.pageContent:createChild("IncludePlugins", {
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
                height = self.get.targetHeight - 54,
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
            buildButton.y = self.get.targetHeight/2 - buildButton.height/2
        end
    end,

    setPage = function(self, pageIndex)
        if pageIndex > self.get.pageIndex then
            if self.get.pageIndex == 1 then
                if self.get.appNameField and self.get.appNameField.get.finalText == "" then
                    self.get.errorMessage.text = Localize:get("error_emptyAppName")
                    self.get.errorMessage.visible = true
                    return false
                end
            elseif self.get.pageIndex == 2 then
                if self.get.orgField and self.get.orgField.get.finalText == "" then
                    self.get.errorMessage.text = Localize:get("error_emptyOrgName")
                    self.get.errorMessage.visible = true
                    return false
                end
                if self.get.appNameField and self.get.appNameField.get.finalText == "" then
                    self.get.errorMessage.text = Localize:get("error_emptyAppName")
                    self.get.errorMessage.visible = true
                    return false
                end
                if self.get.orgField and string.find(self.get.orgField.get.finalText, "[^%a%d_%.]") then
                    self.get.errorMessage.text = Localize:get("error_invalidOrgName")
                    self.get.errorMessage.visible = true
                    return false
                end
                if string.find(self.get.appNameField.get.finalText, "[^%a%d_%.]")then
                    self.get.errorMessage.text = Localize:get("error_invalidAppName")
                    self.get.errorMessage.visible = true
                    return false
                end
                self.get.projectData.android["package-org-name"] = self.get.orgField.get.finalText
                self.get.projectData.android["package-app-name"] = self.get.appNameField.get.finalText
            end
            System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
        
        elseif self.get.pageIndex == 5 then
            self.get.includeFolders.func:confirmOptions()
        elseif self.get.pageIndex == 6 then
            self.get.includePlugins.func:confirmOptions()
    
        end

        if self.get.errorMessage then
            self.get.errorMessage.visible = false
        end
        return self.super.PagedWindow.func:setPage(pageIndex)
    end,

    checkEncryption = function(self)
        self.get.tickBox.frame = self.get.projectData["encryption"] and 2 or 1
    end,

    checkIcon = function(self, path)
        self.get.iconPreview.visible = false
        if self.get.errorMessage then
            self.get.errorMessage.visible = false
        end
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
            if self.get.errorMessage then
                self.get.errorMessage.text = Localize:get("error_invalidIcon")
                self.get.errorMessage.visible = true
            end
            return false
        end
        self.load:image("iconPreview", path)
        self.get.iconPreview.texture = "iconPreview"
        self.get.iconPath = path
        self.get.iconPreview.visible = true
        self.get.iconPreview.rect = {
            self.get.iconPreview.x,
            self.get.iconPreview.y,
            32, 32
        }
        self.get.projectData.android["app-icon"] = self.get.iconPath
        return valid
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
    end,

    startBuilding = function(self)
        self.func:closeWindow(function(win)
            System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
            
            self.world.get.windows:createChild("AndroidBuildNode", {
                projectPath = self.get.projectPath
            })

            win:destroy()
        end)
    end
})