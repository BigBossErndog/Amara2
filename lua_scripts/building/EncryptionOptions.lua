Nodes:define("EncryptionOptions", "UIWindow", {
    width = 256,
    height = 148,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
        if config.projectData then
            self.props.projectData = config.projectData
        end
        if config.returnWindow then
            self.props.returnWindow = config.returnWindow
        end
    end,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.props.tempData = {}
        self.props.tempData["encrypt-lua-scripts"] = true
        self.props.tempData["encrypt-files-assets"] = true
        self.props.tempData["encrypt-write-output"] = true

        if self.props.projectData.encryption then
            for k, v in pairs(self.props.projectData.encryption) do
                self.props.tempData[k] = v
            end
        end

        local title = self.props.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_encryptionOptions"),
            color = Colors.White,
            origin = 0
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        local backButton = self.props.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_cancel",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.props.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.props.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        local removeButton = self.props.content:createChild("UIButton", {
            id = "removeButton",
            text = "label_removeEncryption",
            y = backButton.y,
            onPress = function(button)
                self.func:closeWindow(function(b)
                    self.props.projectData.encryption = nil

                    local newWindow = self.props.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
        removeButton.x = backButton.x - removeButton.width - 4

        local keyTxt = self.props.content:createChild("Text", {
            x = 12, y = title.y + title.height + 10,
            font = "defaultFont",
            text = Localize:get("label_encryptionKey"),
            color = Colors.White,
            origin = 0
        })

        local encryptionKeyField = self.props.content:createChild("TextField", {
            x = keyTxt.x + keyTxt.width + 4, y = title.y + title.height + 8,
            width = self.props.targetWidth - (keyTxt.x + keyTxt.width + 4) - 8,
            defaultText = Localize:get("label_enterEncryptionKey"),
            onCreate = function(self)
                self.classes.TextField.func:onCreate(self)
                self.props.maxTextWidth = self.width - 16
            end,
            onChange = function(textField, txt)
                if self.props.projectData.encryption and self.props.projectData.encryption.key then
                    if txt ~= self.props.projectData.encryption.key then
                        self.props.errorMessage.text = Localize:get("label_warningChangeEncryption")
                        self.props.errorMessage.visible = true
                        self.props.errorMessage.color = Colors.Yellow
                    end
                end
            end
        })
        if self.props.projectData.encryption and self.props.projectData.encryption.key then
            encryptionKeyField.func:setText(self.props.projectData.encryption.key)
        end

        local confirmButton = self.props.content:createChild("UIButton", {
            id = "confirmButton",
            text = "label_confirm",
            onPress = function(button)
                local encryptionKey = encryptionKeyField.props.finalText
                if encryptionKey == "" then
                    self.props.errorMessage.text = Localize:get("error_enterAnEncryptionKey")
                    self.props.errorMessage.visible = true
                    self.props.errorMessage.color = Colors.Red
                    return
                end
                self.props.tempData.key = encryptionKey
                self.props.projectData.encryption = self.props.tempData

                button.props.enabled = false
                self.func:closeWindow(function(b)
                    local newWindow = self.props.returnWindow
                    newWindow.x = self.x
                    newWindow.y = self.y
                    newWindow.func:checkEncryption()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })
        confirmButton.x = self.props.targetWidth - confirmButton.width - 8
        confirmButton.y = self.props.targetHeight - confirmButton.height - 8

        local backer
        backer = self.props.content:createChild("FillRect", {
            x = 8, y = 48,
            origin = 0,
            width = self.props.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.props.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptLuaScripts"),
                })
                backer.props.txt = txt
                backer.props.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.props.tempData["encrypt-lua-scripts"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-lua-scripts")
                    box.props.ticker.frame = self.props.tempData["encrypt-lua-scripts"] and 2 or 1
                    box.props.txt.alpha = self.props.tempData["encrypt-lua-scripts"] and 1 or 0.5
                end
            }
        })
        backer = self.props.content:createChild("FillRect", {
            x = 8, y = 48 + 18,
            origin = 0,
            width = self.props.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.props.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptFilesAndAssets"),
                })
                backer.props.txt = txt
                backer.props.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.props.tempData["encrypt-files-assets"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-files-assets")
                    box.props.ticker.frame = self.props.tempData["encrypt-files-assets"] and 2 or 1
                    box.props.txt.alpha = self.props.tempData["encrypt-files-assets"] and 1 or 0.5
                end
            }
        })
        backer = self.props.content:createChild("FillRect", {
            x = 8, y = 48 + 18*2,
            origin = 0,
            width = self.props.targetWidth - 16,
            height = 18,
            color = "#111d27",
            alpha = self.props.tempData["encrypt-lua-scripts"] and 1 or 0.5,
            onCreate = function(backer)
                local txt = backer:createChild("Text", {
                    x = 8, y = 2, origin = 0,
                    font = "defaultFont", color = Colors.White,
                    text = Localize:get("label_encryptWriteOutput"),
                })
                backer.props.txt = txt
                backer.props.ticker = backer:createChild("Sprite", {
                    origin = 0,
                    x = txt.x + txt.width + 4,
                    y = txt.y + 2,
                    frame = self.props.tempData["encrypt-write-output"] and 2 or 1,
                    texture = "tickBox"
                })
            end,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function(box)
                    self.func:toggleEncryptionOption("encrypt-write-output")
                    box.props.ticker.frame = self.props.tempData["encrypt-write-output"] and 2 or 1
                    box.props.txt.alpha = self.props.tempData["encrypt-write-output"] and 1 or 0.5
                end
            }
        })

        self.props.errorMessage = self.props.content:createChild("Text", {
            x = 10, y = backer.y + backer.height + 2,
            font = "defaultFont",
            text = Localize:get("error_enterAnEncryptionKey"),
            color = Colors.Red,
            visible = false,
            origin = 0,
            wrapMode = WrapMode.ByWord,
            wrapWidth = self.props.targetWidth - 20
        })
    end,

    toggleEncryptionOption = function(self, option)
        self.props.tempData[option] = not self.props.tempData[option]
    end
})