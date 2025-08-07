Nodes:define("InitialSetupWindow", "UIWindow", {
    width = 180,
    height = 42,
    
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
        
        local txt = self.props.content:createChild("Text", {
            text = Localize:get("label_performingInitialSetup"),
            font = "defaultFont",
            color = Colors.White,
            origin = 0
        })
        txt.x = math.floor(self.props.targetWidth/2.0 - txt.width/2.0)
        txt.y = math.floor(self.props.targetHeight/2.0 - txt.height/2.0)

        self.props.loadingBar = self.props.content:createChild("LoadingBar", {
            alpha = 0.3,
            x = 2, y = self.props.targetHeight - 4,
            width = self.props.targetWidth - 4
        })

        local exe = Game.executable

        self.world:hideWindow()
        self.world.forcedClickThrough = true

        self:createChild("ProcessNode", {
            arguments = {
                exe,
                "-context",
                System:getBasePath(),
                "-script",
                "initialSetup/InitialSetup.lua",
                "-display",
                self.world.displayID
            },
            onExit = function(process, exitCode, errorMessage)
                if exitCode ~= 0 then
                    self.func:closeWindow(function()
                        self.world:destroy()
                    end)
                    return
                end
                self.func:closeWindow(function()
                    self.world.func:getSettings(true)
                    self.world.func:fixSettings()

                    local win = self.parent:createChild("MainWindow")
                    win.func:openWindow()

                    self:destroy()
                end)
            end
        })

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})