return NodeFactory:create("MainWindow", "UIWindow", {
    width = 256,
    height = 120,
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
        local props = self.props;

        props.sm = self:createChild("StateMachine")
        
        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.A },
            onPress = function(self)
                self.funcs:exitWindow()
            end
        })
    end,
    exitWindow = function(self)
        self.func:closeBox(function(self)
            self.world:destroy()
        end)
    end,
    createInitialScreen = function(self)
        self.props.content:destroyChildren()

        self.props.content:createChild("Text", {
            x = 8, y = 8,
            font = "defaultFont",
            text = Localize:get("recentProjects_title"),
            color = "#f0f6ff",
            origin = 0,
            input = true
        })

        self.props.content:createChild("FillRect", {
            x = 6, y = 24,
            width = self.width - 12,
            height = 64,
            color = "#111d27",
            origin = 0
        })
        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "Exit",
            x = self.right - self.left - 22,
            y = 4,
            icon = 0,
            onPress = function()
                self.func:exitWindow()
            end
        })
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "Minimize",
            x = self.right - self.left - 40,
            y = 4,
            icon = 3,
            onPress = function(self)
                self.world:minimizeWindow()
            end
        })
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "New Project",
            x = self.right - self.left - 58,
            y = 4,
            icon = 1
        })
    end,
    onUpdate = function(self)
        self.classes.UIWindow.func:onUpdate()

        local sm = self.props.sm

        if sm:state("start") then
            if sm:once() then
                self.func:createInitialScreen()
            end
        end
    end
})