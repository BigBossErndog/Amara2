return Nodes:define("CodeEditorButton", "UIButton", {
    onCreate = function(self)
        local settings = self.world.func:getSettings()
        if settings.codeEditor == "codeEditor_VSCode" then
            self.props.toolTip = "toolTip_openInVSCode"
        elseif settings.codeEditor == "codeEditor_VSCodeInsiders" then
            self.props.toolTip = "toolTip_openInVSCodeInsiders"
        elseif settings.codeEditor == "codeEditor_CodeOSS" then
            self.props.toolTip = "toolTip_openInCodeOSS"
        elseif settings.codeEditor == "codeEditor_Atom" then
            self.props.toolTip = "toolTip_openInAtom"
        elseif settings.codeEditor == "codeEditor_Sublime" then
            self.props.toolTip = "toolTip_openInSublimeText"
        elseif settings.codeEditor == "codeEditor_Sublime-Text" then
            self.props.toolTip = "toolTip_openInSublimeText"
        elseif settings.codeEditor == "codeEditor_Notepad" then
            self.props.toolTip = "toolTip_openInNotepad"
        else
            self.props.toolTip = "toolTip_noCodeEditor"
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
            indexAppend = string.concat(" \"", indexPath, "\"")
        end

        if settings.codeEditor then
            if settings.codeEditor == "codeEditor_VSCode" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
                System:execute(string.concat("code \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_VSCodeInsiders" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
            elseif settings.codeEditor == "codeEditor_CodeOSS" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
                System:execute(string.concat("code-oss \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_Atom" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
                System:execute(string.concat("atom \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_Sublime" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
                System:execute(string.concat("subl \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_Sublime-Text" then
                if projectData.uninitiated then
                    indexAppend = string.concat(" -g \"", indexPath, "\"")
                end
                System:execute(string.concat("sublime_text \"", projectPath, "\"", indexAppend))
            elseif settings.codeEditor == "codeEditor_Notepad" then
                System:execute(string.concat("notepad \"", indexPath, "\""))
            end
        end

        if projectData.uninitiated then
            projectData.uninitiated = nil
            System:writeFile(System:join(projectPath, "project.json"), projectData)
        end
    end
})