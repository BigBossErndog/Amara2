return Nodes:define("InitialSetupWindow", "UIWindow", {
    width = 160,
    height = 48,
    
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

        local exe = Game.executable
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
                    return
                end
                self.func:closeWindow(function()
                    self.world.func:getSettings(true)
                    self.world.func:fixSettings()
                    
                    local settings = self.world.func:getSettings(true)
                    
                    if settings.vsBuildToolsNotInstalled then
                        
                    else
                        local win = self.parent:createChild("MainWindow")
                        win.func:openWindow()
                    end
                end)
            end
        })
    end
})