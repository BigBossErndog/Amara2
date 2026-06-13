Nodes:define("WebBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        else
            return
        end

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        if projectData then
            if projectData["html-name"] then
                self.get.htmlName = projectData["html-name"]
            end
        end

        if (not self.get.htmlName) and projectData["project-name"] then
            self.get.htmlName = projectData["project-name"]
        end

        if config.printLog then
            self.get.printLog = config.printLog
        end

        local args = {}

        local buildDir = System:join(self.get.projectPath, "build", "web")
        self.get.buildDir = buildDir

        -- Clean and create build directory as per Makefile
        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)

        local buildModule
        
        local emscriptenPath
        local pythonPath
        if Game.platform == "windows" then
            buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")
            emscriptenPath = System:join(buildModule, "emsdk/upstream/emscripten")
            pythonPath = System:join(buildModule, "emsdk/python/3.13.3_64bit/python.exe")
        end

        -- Helper to quote paths with spaces
        local function quote_if_needed(path)
            if string.find(path, " ") then
                return '"' .. path .. '"'
            else
                return path
            end
        end

        local function fix_path(path)
            return quote_if_needed(string.gsub(path, "\\", "/"))
        end

        local compilerPath = System:join(emscriptenPath, "em++")
        local buildPath = System:join(buildDir, self.get.htmlName .. ".html")
        self.get.htmlPath = buildPath

        local sdlLibPath = System:join(emscriptenPath, "SDL3", "lib", "libSDL3.a")
        local sdlIncludePath = System:join(emscriptenPath, "SDL3", "include")

        -- table.insert(args, fix_path(compilerPath))
        table.insert(args, fix_path(System:getRelativePath("amara2/main/main.cpp")))

        table.insert(args, fix_path(sdlLibPath))
        table.insert(args, "-I" .. fix_path(sdlIncludePath))

        local static_libs = {}

        -- AMARA_PATH
        table.insert(args, "-Iamara2")
        
        if self.get.projectData["plugin-directories"] and #self.get.projectData["plugin-directories"] > 0 then
            local plugins_path = System:join(self.get.projectPath, "plugins")
            table.insert(args, "-I" .. fix_path(plugins_path))

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
                    
                    if plugin_data["-I"] then
                        for _, path in ipairs(plugin_data["-I"]) do
                            table.insert(args, "-I" .. fix_path(System:join(plugin_path, path)))
                        end
                    end
                    if plugin_data["-L"] then
                        for _, path in ipairs(plugin_data["-L"]) do
                            table.insert(args, "-L" .. fix_path(System:join(plugin_path, path)))
                        end
                    end
                    if plugin_data["-l"] then
                        for _, lib in ipairs(plugin_data["-l"]) do
                            table.insert(args, "-l" .. lib)
                        end
                    end
                    if plugin_data["-l:"] then
                        for _, lib in ipairs(plugin_data["-l:"]) do
                            table.insert(args, "-l:" .. lib)
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

        table.insert(args, "-Isrc")

        if self.get.installPlugins then
            table.insert(args, "-I" .. fix_path(System:join(self.get.projectPath, "plugins")))
        end

        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/json/include")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/lua")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/sol2")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/stb")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/glm")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/minimp3")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/portable-file-dialogs")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/tinyxml2")))
        table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/easy-encryption")))

        -- EMSCRIPTEN_COMPILER_FLAGS (combine -s flags with their values)
        table.insert(args, "-w")
        table.insert(args, "-std=c++17")
        table.insert(args, "-s")
        table.insert(args, "FULL_ES3=1")
        table.insert(args, "-s")
        table.insert(args, "ALLOW_MEMORY_GROWTH=1")
        table.insert(args, "-s")
        table.insert(args, "FORCE_FILESYSTEM=1")
        table.insert(args, "-s")
        table.insert(args, "EXCEPTION_CATCHING_ALLOWED='[\"std::exception\"]'")

        -- EMSCRIPTEN_EXTRA_OPTIONS
        if self.get.installPlugins then
            table.insert(args, "-DAMARA_PLUGINS")
        end
        table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")


        -- EMSCRIPTEN_PRELOADS
        if System:exists(System:join(self.get.projectPath, "lua_scripts")) then
            table.insert(args, "--preload-file")
            table.insert(args, fix_path(System:join(self.get.projectPath, "lua_scripts@/lua_scripts")))
        end
        if System:exists(System:join(self.get.projectPath, "assets")) then
            table.insert(args, "--preload-file")
            table.insert(args, fix_path(System:join(self.get.projectPath, "assets@/assets")))
        end

        if projectData["build-directories"] then
            for i, dir in ipairs(projectData["build-directories"]) do
                if System:exists(System:join(self.get.projectPath, dir)) then
                    table.insert(args, "--preload-file")
                    table.insert(args, fix_path(System:join(self.get.projectPath, dir .. "@/" .. dir)))
                end
            end
        end

        -- Shell
        table.insert(args, "--shell-file")
        table.insert(args, fix_path(System:join(System:getBasePath(), "amara2", "main", "emscripten_shell.html")))

        table.insert(args, "-O2 --closure 1")

        if #static_libs > 0 then
            for _, lib in ipairs(static_libs) do
                table.insert(args, lib)
            end
        end

        -- Output file
        table.insert(args, "-o")
        table.insert(args, fix_path(buildPath))

        local pythonCommand = "set EMSDK_PYTHON=" .. quote_if_needed(pythonPath)
        local argsFile = System:join(buildDir, "build_args.txt")
        System:writeFile(argsFile, string.sep_concat(" ", table.unpack(args)))
        local buildCommand = quote_if_needed(compilerPath) .. " @" .. quote_if_needed(argsFile)
        
        local batchFilePath = System:join(buildDir, "build_web.bat")
        self.get.batchFilePath = batchFilePath
        local errorOutputPath = System:join(buildDir, "build_error.txt")
        local batchFileContent = pythonCommand .. " && " .. buildCommand .. " > " .. fix_path(errorOutputPath) .. " 2>&1 && exit"

        System:writeFile(batchFilePath, batchFileContent)

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

        if not self.get.printLog then
            self.get.printLog = self.world.get.windows:createChild("TerminalWindow", {
                titleText = "title_webBuilder",
                gameProcess = self,
                props = {
                    projectPath = self.get.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,
                onExit = function(self)
                    local newWindow = self.world.get.windows:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })
                    
                    if self.get.gameProcess then
                        System:remove(self.get.gameProcess.get.batchFilePath)
                        System:remove(System:join(self.get.gameProcess.get.buildDir, "build_args.txt"))
                        self.get.gameProcess:destroy()
                        self.get.gameProcess = nil
                    end
                end
            })
            self.get.printLog.func:openWindow()

            self.get.printLog.func:handleMessage(Localize:get("label_building"))
            self.get.printLog.func:handleMessage(Localize:get("label_doNotCloseCommandPrompt"))

            self.get.printLog.func:startLoading()

            self.world:showWindow()
        end
    end,

    -- onOutput = function(self, msg)
    --     if self.get.printLog then
    --         self.get.printLog.func:handleMessage(msg)
    --     end
    -- end,

    onExit = function(self, exitCode)
        System:remove(self.get.batchFilePath)
        System:remove(System:join(self.get.buildDir, "build_args.txt"))

        if self.get.printLog then
            self.get.printLog.func:stopLoading()
            self.get.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        local buildDir = System:join(self.get.projectPath, "build", "windows")
        local errorOutputPath = System:join(buildDir, "build_error.txt")

        if exitCode == 0 then
            if System:exists(errorOutputPath) then
                System:remove(errorOutputPath)
            end
            
            System:rename(self.get.htmlPath, "index")

            -- Notify success
            self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
            System:openDirectory(System:join(self.get.projectPath, "build", "web"))
        else
            local error_message = nil
            if System:exists(errorOutputPath) then
                error_message = System:readFile(errorOutputPath)
                self.get.printLog.func:handleMessage("Error: Build failed.\n" .. error_message)
                System:remove(errorOutputPath)
            end

            -- Notify failure
            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})