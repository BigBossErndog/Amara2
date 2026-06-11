Nodes:define("AndroidBuildOptions", "PagedWindow", {
    id = "buildNode",

    width = 256,
    height = 140,

    props = {
        pageCount = 2
    },

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.exampleProject then
            self.get.exampleProject = config.exampleProject
        end
    end,
    
    onCreate = function(self)
        self.super.PagedWindow.func:onCreate()
        local sdk_config = System:LocateAndroidSDK()
    end
})