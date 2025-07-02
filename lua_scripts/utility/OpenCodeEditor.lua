function OpenCodeEditor(settings, projectPath, filePath)
    local editorConfigs = {
        ["codeEditor_VSCode"]         = { cmd = "code",          project = true, file_flag = "-g" },
        ["codeEditor_VSCodeInsiders"] = { cmd = "code-insiders", project = true, file_flag = "-g" },
        ["codeEditor_CodeOSS"]        = { cmd = "code-oss",      project = true, file_flag = "-g" },
        ["codeEditor_Atom"]           = { cmd = "atom",          project = true, file_flag = nil  },
        ["codeEditor_Sublime"]        = { cmd = "subl",          project = true, file_flag = nil  },
        ["codeEditor_Sublime-Text"]   = { cmd = "sublime_text",  project = true, file_flag = nil  },
        ["codeEditor_Notepad"]        = { cmd = "notepad",       project = false, file_flag = nil }
    }

    local editorKey = settings.codeEditor
    if not editorKey or not editorConfigs[editorKey] then
        return -- No valid editor
    end
    local config = editorConfigs[editorKey]

    local projectData = System:readJSON(System:join(projectPath, "project.json"))
    local isUninitiated = projectData and projectData.uninitiated

    local fileToOpen = filePath
    if not fileToOpen and isUninitiated then
        fileToOpen = System:join(projectPath, "lua_scripts", "index.lua")
    end

    local args = { config.cmd }

    if config.project then
        table.insert(args, '"' .. projectPath .. '"')
    end

    if fileToOpen then
        if config.file_flag then
            table.insert(args, config.file_flag)
            table.insert(args, '"' .. fileToOpen .. '"')
        else
            table.insert(args, '"' .. fileToOpen .. '"')
        end
    end

    local command = table.concat(args, " ")
    System:execute(command)

    if isUninitiated then
        projectData.uninitiated = nil
        System:writeFile(System:join(projectPath, "project.json"), projectData)
    end
end