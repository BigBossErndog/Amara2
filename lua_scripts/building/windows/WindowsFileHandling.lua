local props = Game.args["props"]
local projectData = props.projectData

if props.iconPath then
    System:remove(props.iconPath)
end
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
    local contents = System:getDirectoryContents(string.joinpath(srcDir, dirPath))
    for i, v in ipairs(contents) do
        if System:isDirectory(v) then
            Copy_Travel(srcDir, targetDir, string.joinpath(dirPath, System:getFileName(v)), shouldEncrypt)
        else
            local srcPath = string.joinpath(srcDir, dirPath, System:getFileName(v))
            local targetPath = string.joinpath(targetDir, dirPath, System:getFileName(v))
            
            local hasCompiled = false
            if string.endswith(v, ".lua") and shouldCompile then
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

if System:exists(string.joinpath(props.projectPath, "lua_scripts")) then
    local srcDir = string.joinpath(props.projectPath, "lua_scripts")
    local targetDir = string.joinpath(props.projectPath, "build", "windows", "lua_scripts")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-lua-scripts"))
end

if System:exists(string.joinpath(props.projectPath, "assets")) then
    local srcDir = string.joinpath(props.projectPath, "assets")
    local targetDir = string.joinpath(props.projectPath, "build", "windows", "assets")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
end

if projectData["build-directories"] then
    local directories = projectData["build-directories"]
    for i, v in ipairs(directories) do
        if System:exists(string.joinpath(props.projectPath, v)) then
            local srcDir = string.joinpath(props.projectPath, v)
            local targetDir = string.joinpath(props.projectPath, "build", "windows", System:getDirectoryName(v))
            Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
        end
    end
end