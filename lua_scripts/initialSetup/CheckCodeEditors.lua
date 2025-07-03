local editors = {}

if System:programInstalled("code") then
    table.insert(editors, "codeEditor_VSCode")
end
if System:programInstalled("code-insiders") then
    table.insert(editors, "codeEditor_VSCodeInsiders")
end
if System:programInstalled("code-oss") then
    table.insert(editors, "codeEditor_CodeOSS")
end
if System:programInstalled("atom") then
    table.insert(editors, "codeEditor_Atom")
end
if System:programInstalled("sublime_text") then
    table.insert(editors, "codeEditor_Sublime-Text")
end
if System:programInstalled("subl") then
    table.insert(editors, "codeEditor_Sublime")
end
if System:programInstalled("notepad") then
    table.insert(editors, "codeEditor_Notepad")
end
if System:programInstalled("clion") then
    table.insert(editors, "codeEditor_CLion")
end
if System:programInstalled("cursor") then
    table.insert(editors, "codeEditor_Cursor")
end
if System:programInstalled("zed") then
    table.insert(editors, "codeEditor_Zed")
end
if System:programInstalled("figma") then
    table.insert(editors, "codeEditor_Figma")
end
if System:programInstalled("codium") then
    table.insert(editors, "codeEditor_VSCodium")
end

if #editors > 0 then
    if not Settings.codeEditor then
        Settings.codeEditor = editors[1]
    end
end

if #editors > 0 then
    Settings.codeEditorList = editors
else
    Settings.codeEditorList = {}
end