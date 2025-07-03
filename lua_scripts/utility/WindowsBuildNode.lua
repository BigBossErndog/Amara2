return Nodes:define("WindowsBuildNode", "ProcessNode", {
    id = "buildNode",

    onConfigure = function(self, config)
        if config.projectPath then
            self.props.projectPath = config.projectPath
        else
            self:super_configure(config)
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

        if config.printLog then
            self.props.printLog = config.printLog
        end

        local args = {}

        local buildDir = System:join(self.props.projectPath, "build", "windows")
        self.props.buildDir = buildDir

        local clangLLVMPath = System:getRelativePath("resources/clang-llvm")
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
        local chipmunkPath = System:getRelativePath("resources/libs/Chipmunk2D_build")
        
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

        table.insert(args, System:getRelativePath("resources/clang-llvm/bin/clang++"))
        table.insert(args, System:getRelativePath("amara2/main/main.cpp"))

        if self.props.resOutputFile then
            table.insert(args, self.props.resOutputFile)
        end

        -- AMARA_PATH
        table.insert(args, "-Iamara2")
        if self.props.installPlugins then
            table.insert(args, "-I", System:join(self.props.projectPath, "plugins"))
        end

        -- OTHER_LIB_LINKS
        table.insert(args, System:join(chipmunkPath, "win/chipmunk.lib"))

        -- OTHER_LIB_PATHS
        table.insert(args, "-Isrc")
        table.insert(args, "-I" .. nlohmannPath)
        table.insert(args, "-I" .. murmurhash3Path)
        table.insert(args, "-I" .. luaPath)
        table.insert(args, "-I" .. sol2Path)
        table.insert(args, "-I" .. stbPath)
        table.insert(args, "-I" .. glmPath)
        table.insert(args, "-I" .. System:join(tinyxml2Path, "include"))
        table.insert(args, "-I" .. minimp3Path)
        table.insert(args, "-I" .. pfdPath)
        table.insert(args, "-I" .. tinyxml2Path)
        table.insert(args, "-I" .. System:join(chipmunkPath, "include"))

        -- SDL_PATHS_WIN64
        table.insert(args, "-I" .. System:join(sdl3Path, "include"))
        table.insert(args, "-L" .. System:join(sdl3Path, "lib", "x64"))
        
        -- WINDOWS_COMPILER_FLAGS
        table.insert(args, "-w")
        table.insert(args, "-m64")
        table.insert(args, "-Wl,/SUBSYSTEM:WINDOWS")
        table.insert(args, "-Wl,/NOIMPLIB")
        table.insert(args, "-std=c++17")

        -- EXTRA_OPTIONS
        if self.props.installPlugins then
            table.insert(args, "-DAMARA_PLUGINS")
        end
        table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")
        table.insert(args, "-DAMARA_DESKTOP");

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
            self:super_configure({
                arguments = args
            })
        end
    end,

    onPrepare = function(actor)
        local self = actor:getChild("buildNode")

        actor.world.forcedClickThrough = true

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
        end

        self.props.printLog.func:handleMessage(Localize:get("label_building"))

        if self.props.iconPath then
            System:WriteICO(self.props.iconPath, self.props.iconDest)
            System:writeFile(self.props.resFile, "1 ICON \"" .. self.props.iconDest .. "\"\n")
            
            local command = string.format("%s \"%s\"", System:getRelativePath("resources/clang-llvm/bin/llvm-rc"), self.props.resFile)
            System:execute(command)
        end

        actor.world.forcedClickThrough = false
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
        
        if exitCode == 0 then
            if self.props.iconDest then
                System:remove(self.props.iconDest)
            end
            if self.props.resFile then
                System:remove(self.props.resFile)
            end
            if self.props.resOutputFile then
                System:remove(self.props.resOutputFile)
            end

            System:copy(
                System:join(self.props.projectPath, "lua_scripts"),
                System:join(self.props.projectPath, "build", "windows", "lua_scripts")
            )
            System:copy(
                System:join(self.props.projectPath, "assets"),
                System:join(self.props.projectPath, "build", "windows", "assets")
            )
            if System:exists(System:join(self.props.projectPath, "data")) then
                System:copy(
                    System:join(self.props.projectPath, "data"),
                    System:join(self.props.projectPath, "build", "windows", "data")
                )
            end

            System:openDirectory(System:join(self.props.projectPath, "build", "windows"))

                
            self.props.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
        else
            System:remove(System:join(self.props.projectPath, "build", "windows"))

            self.props.printLog.func:handleMessage(Localize:get("label_buildFailed"))
            if not System:VSBuildToolsInstalled() then
                self.props.printLog.func:handleMessage(Localize:get("error_vsBuildToolsNotFound"))
            end
        end

        self.world.forcedClickThrough = false
    end
})