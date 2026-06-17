local props = System:readJSON(Game.argtable["props_path"])
local projectData = props.projectData

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
    local targetDir = System:join(props.projectPath, "build", "android", "android_package", "assets", "lua_scripts")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-lua-scripts"))
end

if System:exists(System:join(props.projectPath, "assets")) then
    local srcDir = System:join(props.projectPath, "assets")
    local targetDir = System:join(props.projectPath, "build", "android", "android_package", "assets", "assets")
    System:createDirectory(targetDir)
    Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
end

if projectData["build-directories"] then
    local directories = projectData["build-directories"]
    for i, v in ipairs(directories) do
        if System:exists(System:join(props.projectPath, v)) then
            local srcDir = System:join(props.projectPath, v)
            local targetDir = System:join(props.projectPath, "build", "android", "android_package", "assets", System:getDirectoryName(v))
            Copy_Travel(srcDir, targetDir, "", Encrypting("encrypt-files-assets"))
        end
    end
end

local to_inject = {
    "classes.dex",
    "lib/arm64-v8a/libSDL3.so",
    "lib/arm64-v8a/libmain.so",
    "lib/arm64-v8a/libc++_shared.so",
    "lib/armeabi-v7a/libc++_shared.so"
}

local function get_relative_path(base_path, full_path)
    local norm_base = base_path:gsub("\\", "/")
    local norm_full = full_path:gsub("\\", "/")
    
    if not norm_base:sub(-1) == "/" then
        norm_base = norm_base .. "/"
    end
    
    local escaped_base = norm_base:gsub("([%^%$%%%.%*%+%-%?%[%]])", "%%%1")
    local relative = norm_full:gsub("^" .. escaped_base, "")
    relative = relative:gsub("^[/\\]+", "")
    
    return relative
end

local base_path = props.android_package

function ParseAssets(path)
    local contents = System:getDirectoryContents(path)
    for i, v in ipairs(contents) do
        if System:isDirectory(v) then
            ParseAssets(v)
        else
            table.insert(to_inject, get_relative_path(base_path, v))
        end
    end
end
ParseAssets(System:join(base_path, "assets"))

System:InjectIntoAPK(
    System:join(props.android_package, "base.apk"),
    base_path,
    to_inject
)