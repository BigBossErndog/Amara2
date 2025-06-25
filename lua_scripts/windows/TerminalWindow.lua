return Nodes:create("TerminalWindow", "UIWindow", {
    texture = "terminalWindow",

    onConfigure = function(self, config)
        if config.gameProcess then
            self.props.gameProcess = config.gameProcess
        end
    end,

    onCreate = function(self, config)
        self.classes.UIWindow.func:onCreate(self, config)

        if not self.props.poolSize then
            self.props.poolSize = 10
        end

        if self.props.gameProcess then
            self.props.messages = self.props.gameProcess.output
        else
            self.props.messages = {}
        end

        self.props.pool = {}
        for i = 1, self.props.poolSize do
            
        end
    end,

    pipeMessage = function(self, msg)
        table.insert(self.props.messages, msg)
    end,

    unbindGameProcess = function(self)
        self.props.gameProcess = nil
    end
})