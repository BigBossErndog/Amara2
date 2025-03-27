function buildScripts(currentPath, currentRoute)
    local subdirs, files;
    files = files:getFilesInDirectory(currentPath)
    for i, v in ipairs(files) do
        if (v:ends_with(".lua")) then
            local d = files:getRelativePath(currentRoute) .. "/" .. files:removeFileExtension(
                files:getFileName(v)
            ) .. ".luac"
            files:compileScript(v, d)
        end
    end
    subdirs = files:getSubDirectories(currentPath)
    if #subdirs > 0  then
        for i, v in ipairs(subdirs) do
            buildScripts(v, currentRoute .. "/" .. files:getDirectoryName(v))
        end
    end
end