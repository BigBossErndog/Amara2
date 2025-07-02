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

        OpenCodeEditor(settings, projectPath)
    end
})