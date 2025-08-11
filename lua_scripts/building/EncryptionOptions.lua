Nodes:define("EncryptionOptions", "UIWindow", {
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
                self.props.projectData.encryption = {
                    key = encryptionKey
                }
                local encryptionData = self.props.projectData.encryption

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
            height = 48,
            color = "#111d27"
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
    end
})