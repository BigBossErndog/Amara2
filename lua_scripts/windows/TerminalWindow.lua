return Nodes:create("TerminalWindow", "UIWindow", {
    texture = "terminalWindow",

    onConfigure = function(self, config)
        
    end,

    onCreate = function(self, config)
        self.classes.UIWindow.func:onCreate(self, config)
    end
})