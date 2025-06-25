return Nodes:create("CodeEditorButton", "UIButton", {
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

        local projectData = System:readJSON(System:join(projectPath, "project.json"))
        
        local indexAppend = ""
        if projectData.uninitiated then
            indexAppend = string.concat(" -g \"", indexPath, "\"")
        end

        if settings.codeEditor then
            if settings.codeEditor == "codeEditor_VSCode" then
                System:execute(string.concat("code \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_CodeOSS" then
                System:execute(string.concat("code-oss \"", projectPath, "\"", indexAppend))
            end
        end

        if projectData.uninitiated then
            projectData.uninitiated = nil
            System:writeFile(System:join(projectPath, "project.json"), projectData)
        end
    end
})