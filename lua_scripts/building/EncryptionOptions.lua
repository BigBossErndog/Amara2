Nodes:define("EncryptionOptions", "UIWindow", {
    width = 256,
    height = 148,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.projectData then
            self.get.projectData = config.projectData
        end
        if config.returnWindow then
            self.get.returnWindow = config.returnWindow
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.get.tempData = {}
        self.get.tempData["encrypt-lua-scripts"] = true
        self.get.tempData["encrypt-files-assets"] = true
        self.get.tempData["encrypt-write-output"] = true

        if self.get.projectData.encryption then
            for k, v in pairs(self.get.projectData.encryption) do
                self.get.tempData[k] = v
            end
        end

        local title = self.get.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_encryptionOptions"),
            color = Colors.White,
            origin = 0
        })

        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        local backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_cancel",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.get.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        local removeButton = self.get.content:createChild("UIButton", {
            id = "removeButton",
            text = "label_removeEncryption",
            y = backButton.y,
            onPress = function(button)
                self.func:closeWindow(function(b)
                    self.get.projectData.encryption = nil

                    local newWindow = self.get.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
        removeButton.x = backButton.x - removeButton.width - 4

        local keyTxt = self.get.content:createChild("Text", {
            x = 12, y = title.y + title.height + 10,
            font = "defaultFont",
            text = Localize:get("label_encryptionKey"),
            color = Colors.White,
            origin = 0
        })

        local encryptionKeyField = self.get.content:createChild("TextField", {
            x = keyTxt.x + keyTxt.width + 4, y = title.y + title.height + 8,
            width = self.get.targetWidth - (keyTxt.x + keyTxt.width + 4) - 8,
            defaultText = Localize:get("label_enterEncryptionKey"),
            onCreate = function(self)
                self.classes.TextField.func:onCreate(self)
                self.get.maxTextWidth = self.width - 16
            end,
            onChange = function(textField, txt)
                if self.get.projectData.encryption and self.get.projectData.encryption.key then
                    if txt ~= self.get.projectData.encryption.key then
                        self.get.errorMessage.text = Localize:get("label_warningChangeEncryption")
                        self.get.errorMessage.visible = true
                        self.get.errorMessage.color = Colors.Yellow
                    end
                end
            end
        })
        if self.get.projectData.encryption and self.get.projectData.encryption.key then
            encryptionKeyField.func:setText(self.get.projectData.encryption.key)
        end

        local confirmButton = self.get.content:createChild("UIButton", {
            id = "confirmButton",
            text = "label_confirm",
            onPress = function(button)
                local encryptionKey = encryptionKeyField.get.finalText
                if encryptionKey == "" then
                    self.get.errorMessage.text = Localize:get("error_enterAnEncryptionKey")
                    self.get.errorMessage.visible = true
                    self.get.errorMessage.color = Colors.Red
                    return
                end
                self.get.tempData.key = encryptionKey
                self.get.projectData.encryption = self.get.tempData

                button.get.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.get.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
        confirmButton.x = self.get.targetWidth - confirmButton.width - 8
        confirmButton.y = self.get.targetHeight - confirmButton.height - 8

        local backer
        backer = self.get.content:createChild("FillRect", {
            x = 8, y = 48,
            origin = 0,
            width = self.get.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.get.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptLuaScripts"),
                })
                backer.get.txt = txt
                backer.get.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.get.tempData["encrypt-lua-scripts"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-lua-scripts")
                    box.get.ticker.frame = self.get.tempData["encrypt-lua-scripts"] and 2 or 1
                    box.get.txt.alpha = self.get.tempData["encrypt-lua-scripts"] and 1 or 0.5
                end
            }
        })
        backer = self.get.content:createChild("FillRect", {
            x = 8, y = 48 + 18,
            origin = 0,
            width = self.get.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.get.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptFilesAndAssets"),
                })
                backer.get.txt = txt
                backer.get.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.get.tempData["encrypt-files-assets"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-files-assets")
                    box.get.ticker.frame = self.get.tempData["encrypt-files-assets"] and 2 or 1
                    box.get.txt.alpha = self.get.tempData["encrypt-files-assets"] and 1 or 0.5
                end
            }
        })
        backer = self.get.content:createChild("FillRect", {
            x = 8, y = 48 + 18*2,
            origin = 0,
            width = self.get.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.get.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptWriteOutput"),
                })
                backer.get.txt = txt
                backer.get.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.get.tempData["encrypt-write-output"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-write-output")
                    box.get.ticker.frame = self.get.tempData["encrypt-write-output"] and 2 or 1
                    box.get.txt.alpha = self.get.tempData["encrypt-write-output"] and 1 or 0.5
                end
            }
        })

        self.get.errorMessage = self.get.content:createChild("Text", {
            x = 10, y = backer.y + backer.height + 2,
            font = "defaultFont",
            text = Localize:get("error_enterAnEncryptionKey"),
            color = Colors.Red,
            visible = false,
            origin = 0,
            wrapMode = WrapMode.ByWord,
            wrapWidth = self.get.targetWidth - 20
        })
    end,

    toggleEncryptionOption = function(self, option)
        self.get.tempData[option] = not self.get.tempData[option]
    end
})