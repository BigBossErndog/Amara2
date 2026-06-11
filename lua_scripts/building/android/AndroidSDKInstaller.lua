Nodes:define("AndroidSDKInstaller", "UIWindow", {
    width = 256,
    height = 100,

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.sdk then
            self.get.sdk = config.sdk
        end
        if config.ndk then
            self.get.ndk = config.ndk
        end
    end,
    
    onCreate = function(self)
        self.super.UIWindow.func:onCreate()

        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = self.get.sdk and Localize:get("label_androidNDKNotFound") or Localize:get("label_androidSDKNotFound"),
            color = Colors.Yellow,
            origin = 0,
            input = true
        })
        self.get.desc = self.get.content:createChild("Text", {
            x = 10, y = self.get.title.y + self.get.title.height + 8,
            font = "defaultFont",
            text = self.get.sdk and Localize:get("label_androidNDKNotFound_desc") or Localize:get("label_androidSDKNotFound_desc"),
            color = Colors.White,
            origin = 0,
            wrapWidth = self.get.targetWidth - 20
        })

        local buildButton = self.get.content:createChild("UIButton", {
            id = "buildProjectButton",
            text = "label_continue",
            onPress = function()
                local sdk = System:LocateAndroidSDK()
                if sdk then
                    if sdk.ndk then
                        self.func:closeWindow(function(win)
                            self.func:continueBuilding()
                            self:destroy()
                        end)
                    else
                        self.func:closeWindow(function(win)
                            local newWindow = self.world.get.windows:createChild("AndroidSDKInstaller", {
                                projectPath = self.get.projectPath,
                                sdk = sdk
                            })
                            newWindow.func:openWindow()
                            self:destroy()
                        end)
                    end
                else
                    self.func:closeWindow(function(win)
                        local newWindow = self.world.get.windows:createChild("AndroidSDKInstaller", {
                            projectPath = self.get.projectPath
                        })
                        newWindow.func:openWindow()
                        self:destroy()
                    end)
                end
            end
        })
        buildButton.x = self.get.targetWidth - buildButton.width - 8
        buildButton.y = self.get.targetHeight - buildButton.height - 6

        local downloadButton = self.get.content:createChild("UIButton", {
            id = "downloadButton",
            text = Localize:get("label_installAndroidSDK"),
            active = (not self.get.sdk) and true or false,
            onPress = function()
                self.func:downloadAndroidSDK()
            end
        })
        downloadButton.x = buildButton.x - downloadButton.width - 8
        downloadButton.y = buildButton.y
    end,

    continueBuilding = function(self)
        local newWindow = self.world.get.windows:createChild("AndroidBuildOptions", {
            projectPath = self.get.projectPath,
            buildTest = self.get.buildTest
        })
        newWindow.func:openWindow()
    end,

    downloadAndroidSDK = function(self)
        System:openWebsite("https://developer.android.com/studio")
    end
})