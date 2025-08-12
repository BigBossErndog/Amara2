Nodes:define("WindowsBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        else
            return
        end

        if config.installPlugins then
            self.props.installPlugins = config.installPlugins
        end

        local projectData = System:readJSON(System:join(self.props.projectPath, "project.json"))
        if projectData then
            if projectData["executable-name"] then
                self.props.executableName = projectData["executable-name"]
            end
        end

        if (not self.props.executableName) and projectData["project-name"] then
            self.props.executableName = projectData["project-name"]
        end

        if config.printLog then
            self.props.printLog = config.printLog
        end

        local args = {}

        local buildDir = System:join(self.props.projectPath, "build", "windows")
        self.props.buildDir = buildDir

        local clangLLVMPath = System:getRelativePath("build_modules/amara2_windows_build_module/clang-llvm")
        self.props.clangLLVMPath = clangLLVMPath
        
        local sdl3Path = System:getRelativePath("resources/libs/SDL3-3.2.16")

        local nlohmannPath = System:getRelativePath("resources/libs/nlohmann/include")
        local murmurhash3Path = System:getRelativePath("resources/libs/murmurhash3")
        local luaPath = System:getRelativePath("resources/libs/lua")
        local sol2Path = System:getRelativePath("resources/libs/sol2")
        local stbPath = System:getRelativePath("resources/libs/stb")
        local glmPath = System:getRelativePath("resources/libs/glm")
        local minimp3Path = System:getRelativePath("resources/libs/minimp3")
        local pfdPath = System:getRelativePath("resources/libs/portable-file-dialogs")
        local tinyxml2Path = System:getRelativePath("resources/libs/tinyxml2")
        
        -- Clean and create build directory as per Makefile
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

        if config.iconPath then
            self.props.iconPath = config.iconPath
            self.props.iconDest = System:join(buildDir, "icon.ico")
            self.props.resFile = System:join(buildDir, "icon.rc")
            self.props.resOutputFile = System:join(buildDir, "icon.res")
        end

        table.insert(args, System:join(clangLLVMPath, "bin/clang++"))
        table.insert(args, System:getRelativePath("amara2/main/main.cpp"))

        if self.props.resOutputFile then
            table.insert(args, self.props.resOutputFile)
        end

        -- AMARA_PATH
        table.insert(args, "-Iamara2")
        if self.props.installPlugins then
            table.insert(args, "-I", System:join(self.props.projectPath, "plugins"))
        end

        -- OTHER_LIB_PATHS
        table.insert(args, "-Isrc")
        table.insert(args, "-I" .. nlohmannPath)
        table.insert(args, "-I" .. murmurhash3Path)
        table.insert(args, "-I" .. luaPath)
        table.insert(args, "-I" .. sol2Path)
        table.insert(args, "-I" .. stbPath)
        table.insert(args, "-I" .. glmPath)
        table.insert(args, "-I" .. tinyxml2Path)
        table.insert(args, "-I" .. minimp3Path)
        table.insert(args, "-I" .. pfdPath)
        table.insert(args, "-I" .. tinyxml2Path)

        -- SDL_PATHS_WIN64
        table.insert(args, "-I" .. System:join(sdl3Path, "include"))
        table.insert(args, "-L" .. System:join(sdl3Path, "lib", "x64"))
        
        -- WINDOWS_COMPILER_FLAGS
        table.insert(args, "-w")
        table.insert(args, "-m64")
        table.insert(args, "-Wl,/SUBSYSTEM:WINDOWS")
        table.insert(args, "-Wl,/NOIMPLIB")
        table.insert(args, "-std=c++17")

        -- table.insert(args, "-w")
        -- table.insert(args, "-Wall")
        -- table.insert(args, "-std=c++17")
        -- table.insert(args, "-Wl,/NOIMPLIB")
        -- table.insert(args, "-DAMARA_DEBUG_BUILD")

        -- EXTRA_OPTIONS
        if self.props.installPlugins then
            table.insert(args, "-DAMARA_PLUGINS")
        end
        table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")

        -- LINKER_FLAGS_WIN64
        table.insert(args, "-fuse-ld=lld")
        table.insert(args, "-stdlib=libc++")
        table.insert(args, "-L" .. System:join(clangLLVMPath, "lib"))
        table.insert(args, "-pthread")
        table.insert(args, "-DAMARA_OPENGL")
        table.insert(args, "-lopengl32")
        table.insert(args, "-lSDL3")
        table.insert(args, "-lshell32")
        table.insert(args, "-luser32")
        table.insert(args, "-lgdi32")
        table.insert(args, "-lwinmm")
        table.insert(args, "-limm32")
        table.insert(args, "-lole32")
        table.insert(args, "-loleaut32")
        table.insert(args, "-lversion")
        table.insert(args, "-static")

        -- Output file
        table.insert(args, "-o")
        table.insert(args, System:join(buildDir, self.props.executableName .. ".exe"))

        if #args > 0 then
            self:configure({
                arguments = args
            })
        end
    end,

    onPrepare = function(actor)
        local self = actor:getChild("buildNode")

        self.world:hideWindow()

        if self.props.iconPath then
            System:WriteICO(self.props.iconPath, self.props.iconDest)
            System:writeFile(self.props.resFile, "1 ICON \"" .. self.props.iconDest .. "\"\n")
            
            local command = string.format("%s \"%s\"", System:join(self.props.clangLLVMPath, "bin/llvm-rc"), self.props.resFile)
            System:execute(command)
        end

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
                        
                        System:remove(System:join(self.props.projectPath, "build", "windows"))
                    end
                end
            })
            self.props.printLog.func:openWindow()

            self.world:showWindow()
        end

        self.props.printLog.func:startLoading()

        self.props.printLog.func:handleMessage(Localize:get("label_building"))
        self.props.printLog.func:handleMessage(Localize:get("label_doNotCloseCommandPrompt"))
    end,

    onOutput = function(self, msg)
        if self.props.printLog then
            self.props.printLog.func:handleMessage(msg)
        end
    end,

    onExit = function(self, exitCode)
        if self.props.printLog then
            self.props.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        if exitCode == 0 then
            local newProcess = self.parent:createChild("ProcessNode", {
                props = {
                    printLog = self.props.printLog
                },
                arguments = {
                    Game.executable,
                    "-context", System:getBasePath(),
                    "-script", System:getScriptPath("building/windows/WindowsFileHandling"),
                    "-props", self.props
                },
                onOutput = function(self, msg)
                    self.props.printLog.func:handleMessage(msg)
                end,
                onExit = function(self, exitCode)
                    if self.props.printLog then
                        self.props.printLog.func:unbindGameProcess()
                        self.props.printLog.func:stopLoading()
                    end
                    
                    if exitCode == 0 then
                        System:openDirectory(System:join(self.props.projectPath, "build", "windows"))
                        self.props.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
                    else
                        System:remove(System:join(self.props.projectPath, "build", "windows"))
                        self.props.printLog.func:handleMessage("from file handler")
                        self.props.printLog.func:handleMessage(Localize:get("label_buildFailed"))
                        if not System:VSBuildToolsInstalled() then
                            self.props.printLog.func:handleMessage(Localize:get("error_vsBuildToolsNotFound"))
                        end
                    end
                end
            })
            self.props.printLog.props.gameProcess = newProcess
        else
            self.props.printLog.func:stopLoading()
            System:remove(System:join(self.props.projectPath, "build", "windows"))
            
            self.props.printLog.func:handleMessage(Localize:get("label_buildFailed"))
            if not System:VSBuildToolsInstalled() then
                self.props.printLog.func:handleMessage(Localize:get("error_vsBuildToolsNotFound"))
            end
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})