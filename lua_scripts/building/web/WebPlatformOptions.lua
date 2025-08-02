Nodes:define("WebPlatformOptions", "UIWindow", {
    width = 256,
    height = 106,

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        end
    end
})