Nodes:define("WebBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        else
            return
        end

        local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))
        if projectData then
            if projectData["html-name"] then
                self.props.htmlName = projectData["html-name"]
            end
        end

        if (not self.props.htmlName) and projectData["project-name"] then
            self.props.htmlName = projectData["project-name"]
        end

        if config.printLog then
            self.props.printLog = config.printLog
        end

        local args = {}

        local buildDir = System:join(self.props.projectPath, "build", "web")
        self.props.buildDir = buildDir

        -- Clean and create build directory as per Makefile
        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)

        local emscriptenPath
        if Game.platform == "windows" then
            emscriptenPath = System:getRelativePath("build_modules/amara2_windows_build_module/emsdk/upstream/emscripten")
        end

        -- Helper to quote paths with spaces
        local function quote_if_needed(path)
            if string.find(path, " ") then
                return '"' .. path .. '"'
            else
                return path
            end
        end

        local compilerPath = System:join(emscriptenPath, "em++")
        local buildPath = System:join(buildDir, self.props.htmlName .. ".html")
        self.props.htmlPath = buildPath

        local sdlLibPath = System:join(emscriptenPath, "cache", "sysroot", "lib", "libSDL3.a")
        local sdlIncludePath = System:join(emscriptenPath, "cache", "sysroot", "include", "SDL3")

        table.insert(args, quote_if_needed(compilerPath))
        table.insert(args, "./amara2/main/main.cpp")
        table.insert(args, quote_if_needed(sdlLibPath))
        table.insert(args, "-I" .. quote_if_needed(sdlIncludePath))

        -- AMARA_PATH
        table.insert(args, "-Iamara2")

        if (config.installPlugins) then
            self.props.installPlugins = config.installPlugins
        end

        table.insert(args, "-Isrc")

        if self.props.installPlugins then
            table.insert(args, "-I" .. quote_if_needed(System:join(self.props.projectPath, "plugins")))
        end

        table.insert(args, "-Iresources/libs/nlohmann/include")
        table.insert(args, "-Iresources/libs/murmurhash3")
        table.insert(args, "-Iresources/libs/lua")
        table.insert(args, "-Iresources/libs/sol2")
        table.insert(args, "-Iresources/libs/stb")
        table.insert(args, "-Iresources/libs/glm")
        table.insert(args, "-Iresources/libs/minimp3")
        table.insert(args, "-Iresources/libs/portable-file-dialogs")
        table.insert(args, "-Iresources/libs/tinyxml2")

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
        if self.props.installPlugins then
            table.insert(args, "-DAMARA_PLUGINS")
        end
        table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")


        -- EMSCRIPTEN_PRELOADS
        table.insert(args, "--preload-file")
        table.insert(args, quote_if_needed(System:join(self.props.projectPath, "assets@/assets")))
        table.insert(args, "--preload-file")
        table.insert(args, quote_if_needed(System:join(self.props.projectPath, "lua_scripts@/lua_scripts")))
        -- table.insert(args, "--preload-file")
        -- table.insert(args, quote_if_needed(System:join(self.props.projectPath, "data@/data")))


        -- Shell
        table.insert(args, "--shell-file")
        table.insert(args, quote_if_needed(System:join(System:getBasePath(), "amara2", "main", "emscripten_shell.html")))

        table.insert(args, "-O2 --closure 1")

        -- Output file
        table.insert(args, "-o")
        table.insert(args, quote_if_needed(buildPath))

        local buildCommand = string.sep_concat(" ", table.unpack(args))
        local systemCommand = "System:exit(System:executeTerminal(Game.argtable[\"-build-command\"]))"

        if #args > 0 then
            self:configure({
                arguments = {
                    Game.executable,
                    "-context", System:getBasePath(),
                    "-build-command", buildCommand,
                    "-inline-script", systemCommand,
                    "-inline-override"
                }
            })
        end
    end,

    onPrepare = function(actor)
        local self = actor:getChild("buildNode")
        
        self.world:hideWindow()

        if not self.props.printLog then
            self.props.printLog = self.world.props.windows:createChild("TerminalWindow", {
                gameProcess = self,
                props = {
                    projectPath = self.props.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,
                onExit = function(self)
                    local newWindow = self.world.props.windows:createChild("ProjectWindow", {
                        projectPath = self.props.projectPath
                    })
                    newWindow.func:openWindow()
                    
                    if self.props.gameProcess then
                        self.props.gameProcess:destroy()
                        self.props.gameProcess = nil
                    end
                end
            })
            self.props.printLog.func:openWindow()

            self.props.printLog.func:handleMessage(Localize:get("label_building"))
            self.props.printLog.func:handleMessage(Localize:get("label_doNotCloseCommandPrompt"))

            self.props.printLog.func:startLoading()

            self.world:showWindow()
        end
    end,

    onExit = function(self, exitCode)
        if self.props.printLog then
            self.props.printLog.func:stopLoading()
            self.props.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        if exitCode == 0 then
            System:rename(self.props.htmlPath, "index")
            -- Notify success
            self.props.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
            System:openDirectory(System:join(self.props.projectPath, "build", "web"))
        else
            -- Notify failure
            self.props.printLog.func:handleMessage(Localize:get("label_buildFailed"))
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})