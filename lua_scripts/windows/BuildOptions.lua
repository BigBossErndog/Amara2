return Nodes:create("BuildOptions", "UIWindow", {
    width = 256,
    height = 140,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()
    end
})