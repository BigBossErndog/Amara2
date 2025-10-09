local props = Game.argtable["-props"]
local projectData = props.projectData

if props.iconDest then
    System:remove(props.iconDest)
end
if props.resFile then
    System:remove(props.resFile)
end
if props.resOutputFile then
    System:remove(props.resOutputFile)
end

local shouldCompile = false
if projectData["compile-code"] then
    shouldCompile = projectData["compile-code"]
end

local encryption = nil
if projectData.encryption then
    encryption = projectData.encryption
end
function Encrypting(key)
    return encryption and encryption[key]
end

function Copy_Travel(srcDir, targetDir, dirPath, shouldEncrypt)
    local contents = System:getDirectoryContents(System:join(srcDir, dirPath))
    for i, v in ipairs(contents) do
        if System:isDirectory(v) then
            Copy_Travel(srcDir, targetDir, System:join(dirPath, System:getFileName(v)), shouldEncrypt)
        else
            local srcPath = System:join(srcDir, dirPath, System:getFileName(v))
            local targetPath = System:join(targetDir, dirPath, System:getFileName(v))
            
            local hasCompiled = false
            if string.ends_with(v, ".lua") and shouldCompile then
                targetPath = System:removeFileExtension(targetPath) .. ".luac"
                System:compileScript(srcPath, targetPath)
                srcPath = targetPath
                hasCompiled = true
            end

            if shouldEncrypt and encryption then
                System:encryptFile(srcPath, targetPath, encryption.key)
            elseif not hasCompiled then
                System:copy(srcPath, targetPath)
            end
        end
    end
end

if System:exists(System:join(props.projectPath, "lua_scripts")) then
    local srcDir = System:join(props.projectPath, "lua_scripts")
    local targetDir = System:join(props.projectPath, "build", "windows", "lua_scripts")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-lua-scripts"))
end

if System:exists(System:join(props.projectPath, "assets")) then
    local srcDir = System:join(props.projectPath, "assets")
    local targetDir = System:join(props.projectPath, "build", "windows", "assets")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
end

if projectData["build-directories"] then
    local directories = projectData["build-directories"]
    for i, v in ipairs(directories) do
        if System:exists(System:join(props.projectPath, v)) then
            local srcDir = System:join(props.projectPath, v)
            local targetDir = System:join(props.projectPath, "build", "windows", System:getDirectoryName(v))
            Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
        end
    end
end