Nodes:define("WindowsBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        else
            return
        end

        local function quote(path)
            return '"' .. path .. '"'
        end

        local function slash(path)
            return (string.gsub(path, "\\", "/"))
        end

        local function c_string_literal(s)
            s = s:gsub("\\", "\\\\")
            s = s:gsub('"', '\\"')
            s = s:gsub("\r", "\\r"):gsub("\n", "\\n")
            return '"' .. s .. '"'
        end

        local projectData = System:readJSON(string.joinpath(self.get.projectPath, "project.json"))
        if not projectData then
            return
        end
        if projectData["executable-name"] then
            self.get.executableName = projectData["executable-name"]
        end
        self.get.projectData = projectData

        if (not self.get.executableName) and projectData["project-name"] then
            self.get.executableName = projectData["project-name"]
        end

        if config.printLog then
            self.get.printLog = config.printLog
        end

        local args = {}
        local linkArgs = {}

        local buildDir = string.joinpath(self.get.projectPath, "build", "windows")
        if config.buildTest then
            buildDir = string.joinpath(self.get.projectPath, "build", "test")
            self.get.executableName = "Amara2"
            self.get.buildTest = true
        end
        self.get.buildDir = buildDir

        if not self.get.executableName then
            self.get.executableName = "game"
        end

        local buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")
        local resourcePath = System:getRelativePath("resources")

        local winsdk = System:locateWindowsSDK() or {}
        local sdkFound = (winsdk.cl ~= nil) and (winsdk.rc ~= nil)
        local function sdk(key)
            return winsdk[key] or ""
        end

        local sdl3Path = string.joinpath(resourcePath, "libs/SDL3-3.4.10")

        local nlohmannPath = string.joinpath(resourcePath, "libs/json/include")
        local luaPath = string.joinpath(resourcePath, "libs/lua")
        local sol2Path = string.joinpath(resourcePath, "libs/sol2")
        local stbPath = string.joinpath(resourcePath, "libs/stb")
        local glmPath = string.joinpath(resourcePath, "libs/glm")
        local minimp3Path = string.joinpath(resourcePath, "libs/minimp3")
        local pfdPath = string.joinpath(resourcePath, "libs/portable-file-dialogs")
        local tinyxml2Path = string.joinpath(resourcePath, "libs/tinyxml2")
        local minizPath = string.joinpath(resourcePath, "libs/miniz-3.1.1")

        local libDir = System:getRelativePath("lib")
        local hasPlugins = (projectData["plugin-directories"] ~= nil) and (#projectData["plugin-directories"] > 0)
        local useLib =
            not hasPlugins
            and System:exists(string.joinpath(libDir, "amara2.lib"))
        self.get.useLib = useLib

        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)
        System:copy(
            System:getRelativePath("resources/dlls/win64"),
            buildDir
        )

        if not config.iconPath then
            local defaultIcon = System:getRelativePath("assets/icons/icon.png")
            if System:exists(defaultIcon) then
                config.iconPath = defaultIcon
            end
        end

        local hasIcon = (config.iconPath ~= nil) and System:exists(config.iconPath)
        if hasIcon then
            self.get.iconPath = string.joinpath(buildDir, "icon.png")
            self.load:image("exe_icon", config.iconPath)
            Assets:resizeTextureToPNG(
                "exe_icon",
                256, 256,
                self.get.iconPath
            )

            self.get.iconDest = string.joinpath(buildDir, "icon.ico")
            self.get.resFile = string.joinpath(buildDir, "icon.rc")
            self.get.resOutputFile = string.joinpath(buildDir, "icon.res")
        end

        self.get.objFile = string.joinpath(buildDir, "main.obj")
        self.get.minizObjFile = string.joinpath(buildDir, "miniz.obj")

        table.insert(args, "/nologo")
        table.insert(args, fix_path(System:getRelativePath("amara2/main/main.cpp")))
        if not useLib then
            table.insert(args, fix_path(string.joinpath(minizPath, "miniz.c")))
        end

        if self.get.resOutputFile then
            table.insert(args, fix_path(self.get.resOutputFile))
        end

        local static_libs = {}

        table.insert(args, "/I" .. fix_path(sdk("msvc_include")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_ucrt")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_um")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_shared")))

        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("msvc_lib")))
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("sdk_lib_ucrt")))
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("sdk_lib_um")))

        table.insert(args, "/I" .. fix_path(System:getRelativePath("amara2")))

        if hasPlugins then
            local plugins_path = string.joinpath(self.get.projectPath, "plugins")
            table.insert(args, "/I" .. fix_path(plugins_path))

            local plugins = projectData["plugin-directories"]
            local plugin_template = System:readFile(System:getRelativePath("amara2/main/plugin_template.cpp"))

            local includes_str = ""
            local bindings = ""
            local registrations = ""

            for i, plugin in ipairs(plugins) do
                local plugin_path = string.joinpath(plugins_path, plugin)
                local plugin_data = System:readJSON(string.joinpath(plugin_path, "plugin.json"))
                if plugin_data then
                    if plugin_data.includes then
                        for _, file in ipairs(plugin_data.includes) do
                            includes_str = includes_str .. "#include \"" .. plugin .. "/" .. file .. "\"\n"
                        end
                    end
                    if plugin_data.nodes then
                        for _, node_data in ipairs(plugin_data.nodes) do
                            bindings = bindings .. node_data.class .. "::bind_lua(lua);"
                            registrations = registrations .. "registerNode<" .. node_data.class .. ">(\"" .. node_data.nodeID .. "\");"
                        end
                    end
                    if plugin_data.copy then
                        for _, file in ipairs(plugin_data.copy) do
                            System:copy(string.joinpath(plugin_path, file), buildDir)
                        end
                    end

                    if plugin_data["-I"] then
                        for _, path in ipairs(plugin_data["-I"]) do
                            table.insert(args, "/I" .. fix_path(string.joinpath(plugin_path, path)))
                        end
                    end
                    if plugin_data["-L"] then
                        for _, path in ipairs(plugin_data["-L"]) do
                            table.insert(linkArgs, "/LIBPATH:" .. fix_path(string.joinpath(plugin_path, path)))
                        end
                    end
                    if plugin_data["-l"] then
                        for _, lib in ipairs(plugin_data["-l"]) do
                            table.insert(linkArgs, lib .. ".lib")
                        end
                    end
                    if plugin_data["-l:"] then
                        for _, lib in ipairs(plugin_data["-l:"]) do
                            table.insert(linkArgs, lib)
                        end
                    end
                    if plugin_data[".lib"] then
                        for _, lib in ipairs(plugin_data[".lib"]) do
                            if not string.endswith(lib, ".lib") then
                                lib = lib .. ".lib"
                            end
                            table.insert(static_libs, fix_path(string.joinpath(plugin_path, lib)))
                        end
                    end
                end
                local copy_path = string.joinpath(plugin_path, "copy")
                if System:directoryExists(copy_path) then
                    local contents = System:getDirectoryContents(copy_path)
                    for _, file in ipairs(contents) do
                        System:copy(file, buildDir)
                    end
                end
            end

            plugin_template = plugin_template:gsub("// plugin_includes", function() return includes_str end)
            plugin_template = plugin_template:gsub("// plugin_lua_bindings", function() return bindings end)
            plugin_template = plugin_template:gsub("// plugin_node_registrations", function() return registrations end)

            System:writeFile(string.joinpath(plugins_path, "amara2_plugins.cpp"), plugin_template)
        end

        if not useLib then
            table.insert(args, "/Isrc")
            table.insert(args, "/I" .. fix_path(nlohmannPath))
            table.insert(args, "/I" .. fix_path(luaPath))
            table.insert(args, "/I" .. fix_path(sol2Path))
            table.insert(args, "/I" .. fix_path(stbPath))
            table.insert(args, "/I" .. fix_path(glmPath))
            table.insert(args, "/I" .. fix_path(tinyxml2Path))
            table.insert(args, "/I" .. fix_path(minimp3Path))
            table.insert(args, "/I" .. fix_path(pfdPath))
            table.insert(args, "/I" .. fix_path(minizPath))

            table.insert(args, "/I" .. fix_path(string.joinpath(sdl3Path, "include")))
            table.insert(linkArgs, "/LIBPATH:" .. fix_path(string.joinpath(sdl3Path, "lib", "x64")))
        end

        table.insert(args, "/w")
        table.insert(args, "/std:c++17")
        table.insert(args, "/EHsc")
        table.insert(args, "/O2")
        table.insert(args, "/MT")
        table.insert(args, "/utf-8")
        table.insert(args, "/bigobj")
        table.insert(args, "/Zc:__cplusplus")
        table.insert(linkArgs, "/SUBSYSTEM:WINDOWS")

        if useLib then
            table.insert(linkArgs, "/LIBPATH:" .. fix_path(libDir))
        else
            table.insert(args, "/DAMARA_IMPLEMENTATION")
        end

        if hasPlugins then
            table.insert(args, "/DAMARA_PLUGINS")
        end
        if not config.buildTest then
            if not useLib then
                table.insert(args, "/DAMARA_DISABLE_EXTERNAL_SCRIPTS")
            end
        else
            table.insert(args, "/DAMARA_DEBUGGING")
            table.insert(args, "/DAMARA_ENGINE_TOOLS")
        end

        if self.get.projectData.build_args then
            local build_args = {}
            for k, v in pairs(self.get.projectData.build_args) do
                if v then
                    build_args[k] = string.acceptjson(v) and string.parsejson(v) or true
                else
                    build_args[k] = true
                end
            end
            local args_as_str = string.jsonstring(build_args)
            local args_macro = quote_bat_str(args_as_str)
            table.insert(args, "/DAMARA_BUILD_ARGS=" .. args_macro)
        end

        if self.get.projectData.encryption and not config.buildTest then
            table.insert(args, "/DAMARA_ENCRYPTION_KEY=" .. quote_if_needed(self.get.projectData.encryption["key"]))
            if self.get.projectData.encryption["encrypt-write-output"] then
                table.insert(args, "/DAMARA_ENCRYPT_OUTPUT")
            end
        end

        if not useLib then
            table.insert(args, "/DAMARA_OPENGL")
            table.insert(linkArgs, "opengl32.lib")
            table.insert(linkArgs, "SDL3.lib")
            table.insert(linkArgs, "shell32.lib")
            table.insert(linkArgs, "user32.lib")
            table.insert(linkArgs, "gdi32.lib")
            table.insert(linkArgs, "winmm.lib")
            table.insert(linkArgs, "imm32.lib")
            table.insert(linkArgs, "ole32.lib")
            table.insert(linkArgs, "oleaut32.lib")
            table.insert(linkArgs, "version.lib")
        end

        if #static_libs > 0 then
            for _, lib in ipairs(static_libs) do
                table.insert(linkArgs, lib)
            end
        end

        table.insert(args, "/Fo:" .. quote(slash(buildDir) .. "/"))
        table.insert(args, "/Fe:" .. quote(slash(string.joinpath(buildDir, self.get.executableName .. ".exe"))))

        table.insert(args, "/link")
        for _, a in ipairs(linkArgs) do
            table.insert(args, a)
        end

        local argsFile = string.joinpath(buildDir, "build_args.txt")
        System:writeFile(argsFile, table.concat(args, " "))

        local batchFilePath = string.joinpath(buildDir, "build_windows.bat")
        self.get.batchFilePath = batchFilePath
        local errorOutputPath = string.joinpath(buildDir, "build_error.txt")
        self.get.errorOutputPath = errorOutputPath
        local errorLog = quote(slash(errorOutputPath))

        local clCommand = quote(slash(sdk("cl"))) .. " @" .. quote(slash(argsFile))

        local batchLines = {}
        if sdkFound then
            local logRedirect = " > "

            if hasIcon then
                local rcCommand = quote(slash(sdk("rc"))) .. " /nologo"
                    .. " /I" .. quote(slash(sdk("sdk_include_um")))
                    .. " /I" .. quote(slash(sdk("sdk_include_shared")))
                    .. " /fo " .. quote(slash(self.get.resOutputFile))
                    .. " " .. quote(slash(self.get.resFile))
                table.insert(batchLines, rcCommand .. logRedirect .. errorLog .. " 2>&1 || exit 1")
                logRedirect = " >> "
            end

            table.insert(batchLines, clCommand .. logRedirect .. errorLog .. " 2>&1 || exit 1")
            table.insert(batchLines, "exit 0")
        else
            table.insert(batchLines, "echo MSVC build tools or the Windows SDK could not be found. > " .. errorLog)
            table.insert(batchLines, "exit 1")
        end

        System:writeFile(batchFilePath, table.concat(batchLines, "\r\n") .. "\r\n")

        local systemCommand = "System:exit(System:executeTerminal(" .. string.format("%q", quote_if_needed(batchFilePath)) .. "))"

        if #args > 0 then
            self:configure({
                arguments = {
                    Game.executable,
                    "-context", System:getBasePath(),
                    "-inline-script", systemCommand,
                    "-inline-override"
                }
            })
        end
    end,

    onPrepare = function(actor)
        local self = actor:getChild("buildNode")

        self.world:hideWindow()

        if self.get.iconPath then
            System:writeICO(self.get.iconPath, self.get.iconDest)
            local iconDestForRc = string.gsub(self.get.iconDest, "\\", "/")
            System:writeFile(self.get.resFile, "1 ICON \"" .. iconDestForRc .. "\"\n")
        end

        if not self.get.printLog then
            self.get.printLog = self.world.get.windows:createChild("TerminalWindow", {
                titleText = "title_windowsBuilder",
                gameProcess = self,
                props = {
                    projectPath = self.get.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,
                onExit = function(self)
                    if self.get.gameProcess then
                        System:remove(self.get.gameProcess.get.batchFilePath)
                        System:remove(string.joinpath(self.get.gameProcess.get.buildDir, "build_args.txt"))
                    end

                    local newWindow = self.world.get.windows:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })

                    if self.get.gameProcess then
                        self.get.gameProcess:destroy()
                        self.get.gameProcess = nil
                    end
                end
            })
            self.get.printLog.func:openWindow()

            self.world:showWindow()
        end

        self.get.printLog.func:startLoading()

        self.get.printLog.func:handleMessage(Localize:get("label_building"))
        self.get.printLog.func:handleMessage(Localize:get("label_doNotCloseCommandPrompt"))
    end,

    onExit = function(self, exitCode)
        System:remove(self.get.batchFilePath)
        System:remove(string.joinpath(self.get.buildDir, "build_args.txt"))
        if self.get.objFile and System:exists(self.get.objFile) then
            System:remove(self.get.objFile)
        end
        if self.get.minizObjFile and System:exists(self.get.minizObjFile) then
            System:remove(self.get.minizObjFile)
        end

        if self.get.printLog then
            self.get.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        local buildDir = self.get.buildDir
        local errorOutputPath = self.get.errorOutputPath

        local function removeIconFiles()
            if self.get.iconPath and System:exists(self.get.iconPath) then
                System:remove(self.get.iconPath)
            end
            if self.get.iconDest and System:exists(self.get.iconDest) then
                System:remove(self.get.iconDest)
            end
            if self.get.resFile and System:exists(self.get.resFile) then
                System:remove(self.get.resFile)
            end
            if self.get.resOutputFile and System:exists(self.get.resOutputFile) then
                System:remove(self.get.resOutputFile)
            end
        end

        if exitCode == 0 then
            if System:exists(errorOutputPath) then
                System:remove(errorOutputPath)
            end
            if not self.get.buildTest then
                local newProcess = self.parent:createChild("ProcessNode", {
                    props = {
                        projectPath = self.get.projectPath,
                        printLog = self.get.printLog
                    },
                    arguments = {
                        Game.executable,
                        "-context", System:getBasePath(),
                        "-script", System:getScriptPath("building/windows/WindowsFileHandling"),
                        "-props", self.props
                    },
                    onOutput = function(self, msg)
                        self.get.printLog.func:handleMessage(msg)
                    end,
                    onExit = function(self, exitCode)
                        if self.get.printLog then
                            self.get.printLog.func:unbindGameProcess()
                            self.get.printLog.func:stopLoading()
                        end

                        if exitCode == 0 then
                            System:openDirectory(string.joinpath(self.get.projectPath, "build", "windows"))
                            self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
                        else
                            System:remove(string.joinpath(self.get.projectPath, "build", "windows"))
                            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))
                        end
                    end
                })
                self.get.printLog.get.gameProcess = newProcess
            else
                if self.get.printLog then
                    self.get.printLog.func:unbindGameProcess()
                    self.get.printLog.func:stopLoading()
                end
                self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))

                removeIconFiles()
            end

        else
            self.get.printLog.func:stopLoading()

            local error_message = nil
            if System:exists(errorOutputPath) then
                error_message = System:readFile(errorOutputPath)
                self.get.printLog.func:handleMessage("Error: Build failed.\n" .. error_message)
                System:remove(errorOutputPath)
            end

            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))

            if not System:installedVSBuildTools() then
                self.get.printLog.func:handleMessage(Localize:get("error_vsBuildToolsNotFound"))
            end

            removeIconFiles()
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})