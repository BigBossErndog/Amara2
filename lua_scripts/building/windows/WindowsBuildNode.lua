Nodes:define("WindowsBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        else
            return
        end

        local function quote_if_needed(path)
            if string.find(path, " ") then
                return '"' .. path .. '"'
            else
                return path
            end
        end

        -- Always quotes (used for the .bat file, where tool paths nearly always contain spaces)
        local function quote(path)
            return '"' .. path .. '"'
        end

        -- Backslashes to forward slashes (parentheses drop gsub's second return value)
        local function slash(path)
            return (string.gsub(path, "\\", "/"))
        end

        local function fix_path(path)
            return quote_if_needed(slash(path))
        end

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        if projectData then
            if projectData["executable-name"] then
                self.get.executableName = projectData["executable-name"]
            end
        end
        self.get.projectData = projectData

        if (not self.get.executableName) and projectData["project-name"] then
            self.get.executableName = projectData["project-name"]
        end

        if config.printLog then
            self.get.printLog = config.printLog
        end

        -- args: compiler options. linkArgs: everything after /link
        local args = {}
        local linkArgs = {}

        local buildDir = System:join(self.get.projectPath, "build", "windows")
        if config.buildTest then
            buildDir = System:join(self.get.projectPath, "build", "test")
            self.get.executableName = "Amara2"
            self.get.buildTest = true
        end
        self.get.buildDir = buildDir

        local buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")

        -- MSVC / Windows SDK locations (from the C++ locateWindowsSDK function)
        local winsdk = System:locateWindowsSDK() or {}
        local sdkFound = (winsdk.cl ~= nil) and (winsdk.rc ~= nil)
        local function sdk(key)
            return winsdk[key] or ""
        end

        local sdl3Path = System:join(buildModule, "resources/libs/SDL3-3.4.10")

        local nlohmannPath = System:join(buildModule, "resources/libs/json/include")
        local luaPath = System:join(buildModule, "resources/libs/lua")
        local sol2Path = System:join(buildModule, "resources/libs/sol2")
        local stbPath = System:join(buildModule, "resources/libs/stb")
        local glmPath = System:join(buildModule, "resources/libs/glm")
        local minimp3Path = System:join(buildModule, "resources/libs/minimp3")
        local pfdPath = System:join(buildModule, "resources/libs/portable-file-dialogs")
        local tinyxml2Path = System:join(buildModule, "resources/libs/tinyxml2")
        
        -- Clean and create build directory as per Makefile
        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)
        System:copy(
            System:join(buildModule, "resources/dlls/win64"),
            buildDir
        )
        
        if not config.iconPath then
            local defaultIcon = System:getRelativePath("assets/icons/icon.png")
            if System:exists(defaultIcon) then
                config.iconPath = defaultIcon
            end
        end
        self.get.iconPath = System:join(buildDir, "icon.png")
        self.load:image("exe_icon", config.iconPath)
        Assets:resizeTextureToPNG(
            "exe_icon",
            256, 256,
            self.get.iconPath
        )
        
        self.get.iconDest = System:join(buildDir, "icon.ico")
        self.get.resFile = System:join(buildDir, "icon.rc")
        self.get.resOutputFile = System:join(buildDir, "icon.res")

        self.get.objFile = System:join(buildDir, "main.obj")

        -- Compiler options
        table.insert(args, "/nologo")
        table.insert(args, fix_path(System:getRelativePath("amara2/main/main.cpp")))
        
        if self.get.resOutputFile then
            table.insert(args, fix_path(self.get.resOutputFile))
        end

        local static_libs = {}

        -- MSVC and Windows SDK headers
        table.insert(args, "/I" .. fix_path(sdk("msvc_include")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_ucrt")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_um")))
        table.insert(args, "/I" .. fix_path(sdk("sdk_include_shared")))

        -- MSVC and Windows SDK import libraries
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("msvc_lib")))
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("sdk_lib_ucrt")))
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(sdk("sdk_lib_um")))

        -- AMARA_PATH
        table.insert(args, "/I" ..  fix_path(System:getRelativePath("amara2")))
        
        if self.get.projectData["plugin-directories"] and #self.get.projectData["plugin-directories"] > 0 then
            local plugins_path = System:join(self.get.projectPath, "plugins")
            table.insert(args, "/I" .. fix_path(plugins_path))

            local plugins = self.get.projectData["plugin-directories"]
            local plugin_template = System:readFile(System:getRelativePath("amara2/main/plugin_template.cpp"))
            for i, plugin in ipairs(plugins) do
                local plugin_path = System:join(plugins_path, plugin)
                local plugin_data = System:readJSON(System:join(plugin_path, "plugin.json"))
                if plugin_data then
                    if plugin_data.includes then
                        local includes_str = ""
                        for _, file in ipairs(plugin_data.includes) do
                            includes_str = includes_str .. "#include \"" .. System:join(plugin, file) .. "\"\n"
                        end
                        plugin_template = string.gsub(plugin_template, "// plugin_includes", includes_str)
                    end
                    if plugin_data.nodes then
                        local bindings = ""
                        local registrations = ""
                        for _, node_data in ipairs(plugin_data.nodes) do
                            bindings = bindings .. node_data.class .. "::bind_lua(lua);"
                            registrations = registrations .. "registerNode<" .. node_data.class .. ">(\"" .. node_data.nodeID .. "\");"
                        end
                        plugin_template = string.gsub(plugin_template, "// plugin_lua_bindings", bindings)
                        plugin_template = string.gsub(plugin_template, "// plugin_node_registrations", registrations)
                    end
                    if plugin_data.copy then
                        for _, file in ipairs(plugin_data.copy) do
                            System:copy(System:join(plugin_path, file), buildDir)
                        end
                    end
                    
                    -- plugin.json keeps its clang-style keys; they are translated to MSVC options here
                    if plugin_data["-I"] then
                        for _, path in ipairs(plugin_data["-I"]) do
                            table.insert(args, "/I" .. fix_path(System:join(plugin_path, path)))
                        end
                    end
                    if plugin_data["-L"] then
                        for _, path in ipairs(plugin_data["-L"]) do
                            table.insert(linkArgs, "/LIBPATH:" .. fix_path(System:join(plugin_path, path)))
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
                            if not string.ends_with(lib, ".lib") then
                                lib = lib .. ".lib"
                            end
                            table.insert(static_libs, fix_path(System:join(plugin_path, lib)))
                        end
                    end
                end
                local copy_path = System:join(plugin_path, "copy")
                if System:directoryExists(copy_path) then
                    local contents = System:getDirectoryContents(copy_path)
                    for _, file in ipairs(contents) do
                        System:copy(file, buildDir)
                    end
                end
            end

            System:writeFile(System:join(plugins_path, "amara2_plugins.cpp"), plugin_template)
        end

        -- OTHER_LIB_PATHS
        table.insert(args, "/Isrc")
        table.insert(args, "/I" .. fix_path(nlohmannPath))
        table.insert(args, "/I" .. fix_path(luaPath))
        table.insert(args, "/I" .. fix_path(sol2Path))
        table.insert(args, "/I" .. fix_path(stbPath))
        table.insert(args, "/I" .. fix_path(glmPath))
        table.insert(args, "/I" .. fix_path(tinyxml2Path))
        table.insert(args, "/I" .. fix_path(minimp3Path))
        table.insert(args, "/I" .. fix_path(pfdPath))

        -- SDL_PATHS_WIN64
        table.insert(args, "/I" .. fix_path(System:join(sdl3Path, "include")))
        table.insert(linkArgs, "/LIBPATH:" .. fix_path(System:join(sdl3Path, "lib", "x64")))
        
        -- WINDOWS_COMPILER_FLAGS
        table.insert(args, "/w")
        table.insert(args, "/std:c++17")
        table.insert(args, "/EHsc")
        table.insert(args, "/O2")
        table.insert(args, "/MT")                -- static runtime (replaces -static)
        table.insert(args, "/utf-8")             -- clang assumes UTF-8 sources; MSVC does not
        table.insert(args, "/bigobj")            -- sol2 translation units can exceed the default object limit
        table.insert(args, "/Zc:__cplusplus")
        table.insert(linkArgs, "/SUBSYSTEM:WINDOWS")
        -- If linking fails with "unresolved external symbol WinMain", the entry point is `main`:
        -- table.insert(linkArgs, "/ENTRY:mainCRTStartup")
        
        -- EXTRA_OPTIONS
        if self.get.projectData["plugin-directories"] and #self.get.projectData["plugin-directories"] > 0 then
            table.insert(args, "/DAMARA_PLUGINS")
        end
        if not config.buildTest then
            table.insert(args, "/DAMARA_DISABLE_EXTERNAL_SCRIPTS")
        else
            table.insert(args, "/DAMARA_DEBUGGING")
            table.insert(args, "/DAMARA_ENGINE_TOOLS")
        end

        if self.get.projectData.encryption and not config.buildTest then
            table.insert(args, "/DAMARA_ENCRYPTION_KEY=" .. quote_if_needed(self.get.projectData.encryption["key"]))
            if self.get.projectData.encryption["encrypt-write-output"] then
                table.insert(args, "/DAMARA_ENCRYPT_OUTPUT")
            end
        end

        -- LINKER_FLAGS_WIN64
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

        if #static_libs > 0 then
            for _, lib in ipairs(static_libs) do
                table.insert(linkArgs, lib)
            end
        end

        -- Output files (object file goes into the build dir; it is removed after the build)
        table.insert(args, "/Fo:" .. quote(slash(self.get.objFile)))
        table.insert(args, "/Fe:" .. quote(slash(System:join(buildDir, self.get.executableName .. ".exe"))))

        -- Everything after /link goes to the linker
        table.insert(args, "/link")
        for _, a in ipairs(linkArgs) do
            table.insert(args, a)
        end

        local argsFile = System:join(buildDir, "build_args.txt")
        System:writeFile(argsFile, string.sep_concat(" ", table.unpack(args)))

        local batchFilePath = System:join(buildDir, "build_windows.bat")
        self.get.batchFilePath = batchFilePath
        local errorOutputPath = System:join(buildDir, "build_error.txt")
        self.get.errorOutputPath = errorOutputPath
        local errorLog = quote(slash(errorOutputPath))

        -- Step 1: compile the icon resource with rc (needs the SDK headers only if the .rc includes any)
        local rcCommand = quote(slash(sdk("rc"))) .. " /nologo"
            .. " /I" .. quote(slash(sdk("sdk_include_um")))
            .. " /I" .. quote(slash(sdk("sdk_include_shared")))
            .. " /fo " .. quote(slash(self.get.resOutputFile))
            .. " " .. quote(slash(self.get.resFile))

        -- Step 2: compile and link with cl, using the response file
        local clCommand = quote(slash(sdk("cl"))) .. " @" .. quote(slash(argsFile))

        local batchFileContent
        if sdkFound then
            batchFileContent = rcCommand .. " > " .. errorLog .. " 2>&1"
                .. " && " .. clCommand .. " >> " .. errorLog .. " 2>&1"
                .. " && exit"
        else
            batchFileContent = "echo MSVC build tools or the Windows SDK could not be found. > " .. errorLog
                .. " & exit /b 1"
        end

        System:writeFile(batchFilePath, batchFileContent)

        local systemCommand = "System:exit(System:executeTerminal(" .. string.format("%q", quote_if_needed(batchFilePath)) .. "))"

        if #args > 0 then
            self:configure({
                -- arguments = args
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
            -- The .ico is written here; rc.exe itself runs as the first step of the build .bat file.
            -- Forward slashes keep rc from treating backslashes in the path as escape characters.
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
                        System:remove(System:join(self.get.gameProcess.get.buildDir, "build_args.txt"))
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
        System:remove(System:join(self.get.buildDir, "build_args.txt"))
        if self.get.objFile and System:exists(self.get.objFile) then
            System:remove(self.get.objFile)
        end
        
        if self.get.printLog then
            self.get.printLog.func:unbindGameProcess()
        end
        
        self.world.forcedClickThrough = true
        self.world:hideWindow()

        local buildDir = self.get.buildDir
        local errorOutputPath = self.get.errorOutputPath

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
                            System:openDirectory(System:join(self.get.projectPath, "build", "windows"))
                            self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
                        else
                            System:remove(System:join(self.get.projectPath, "build", "windows"))
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
                System:openDirectory(System:join(self.get.buildDir))
                self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))

                if self.get.iconPath then
                    System:remove(self.get.iconPath)
                end
                if self.get.iconDest then
                    System:remove(self.get.iconDest)
                end
                if self.get.resFile then
                    System:remove(self.get.resFile)
                end
                if self.get.resOutputFile then
                    System:remove(self.get.resOutputFile)
                end
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
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})