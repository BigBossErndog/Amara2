function OpenCodeEditor(settings, projectPath, filePath)
    local indexPath = System:join(projectPath, "lua_scripts", "index.lua")

    local projectData = System:readJSON(System:join(projectPath, "project.json"))
    
    local indexAppend = ""
    if filePath then
        indexAppend = string.concat(" \"", filePath, "\"")
    elseif projectData.uninitiated then
        indexAppend = string.concat(" \"", indexPath, "\"")
    end

    if settings.codeEditor then
        if settings.codeEditor == "codeEditor_VSCode" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
            System:execute(string.concat("code \"", projectPath, "\"", indexAppend))
        elseif settings.codeEditor == "codeEditor_VSCodeInsiders" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
        elseif settings.codeEditor == "codeEditor_CodeOSS" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
            System:execute(string.concat("code-oss \"", projectPath, "\"", indexAppend))
        elseif settings.codeEditor == "codeEditor_Atom" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
            System:execute(string.concat("atom \"", projectPath, "\"", indexAppend))
        elseif settings.codeEditor == "codeEditor_Sublime" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
            System:execute(string.concat("subl \"", projectPath, "\"", indexAppend))
        elseif settings.codeEditor == "codeEditor_Sublime-Text" then
            if projectData.uninitiated and not filePath then
                indexAppend = string.concat(" -g \"", indexPath, "\"")
            end
            System:execute(string.concat("sublime_text \"", projectPath, "\"", indexAppend))
        elseif settings.codeEditor == "codeEditor_Notepad" then
            System:execute(string.concat("notepad \"", indexPath, "\""))
        end
    end

    if projectData.uninitiated and not filePath then
        projectData.uninitiated = nil
        System:writeFile(System:join(projectPath, "project.json"), projectData)
    end
end