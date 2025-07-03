local editorTooltips = {
    ["codeEditor_VSCode"] = "toolTip_openInVSCode",
    ["codeEditor_VSCodeInsiders"] = "toolTip_openInVSCodeInsiders",
    ["codeEditor_CodeOSS"] = "toolTip_openInCodeOSS",
    ["codeEditor_Atom"] = "toolTip_openInAtom",
    ["codeEditor_Sublime"] = "toolTip_openInSublimeText",
    ["codeEditor_Sublime-Text"] = "toolTip_openInSublimeText",
    ["codeEditor_Notepad"] = "toolTip_openInNotepad",
    ["codeEditor_CLion"] = "toolTip_openInCLion",
    ["codeEditor_Cursor"] = "toolTip_openInCursor",
    ["codeEditor_Zed"] = "toolTip_openInZed",
    ["codeEditor_Figma"] = "toolTip_openInFigma",
    ["codeEditor_VSCodium"] = "toolTip_openInVSCodium"
}

return Nodes:define("CodeEditorButton", "UIButton", {
    onCreate = function(self)
        local settings = self.world.func:getSettings()
        
        if settings.codeEditor and editorTooltips[settings.codeEditor] then
            self.props.toolTip = editorTooltips[settings.codeEditor]
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