return NodeFactory:create("CodeEditorButton", "UIButton", {
    onCreate = function(self)
        local settings = self.world.func:getSettings()
        if settings.codeEditor == "codeEditor_VSCode" then
            self.props.toolTip = "toolTip_openInVSCode"
        elseif settings.codeEditor == "codeEditor_CodeOSS" then
            self.props.toolTip = "toolTip_openInCodeOSS"
        else
            self.props.toolTip = "toolTip_openInCodeEditor"
        end

        self.classes.UIButton.func:onCreate()
    end,

    onPress = function(self)
        local settings = self.world.func:getSettings()

        local projectPath = self.parent.parent.props.projectPath
        local indexPath = System:join(projectPath, "lua_scripts", "index.lua")

        if settings.codeEditor then
            if settings.codeEditor == "codeEditor_VSCode" then
                System:execute(string.concat("code \"", projectPath, "\" -g \"", indexPath, "\""))
            elseif settings.codeEditor == "codeEditor_CodeOSS" then
                System:execute(string.concat("code-oss \"", projectPath, "\" -g \"", indexPath, "\""))
            end
        end
    end
})