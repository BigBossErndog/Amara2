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

        local function fix_path(path)
            return quote_if_needed(string.gsub(path, "\\", "/"))
        end

        if config.installPlugins then
            self.get.installPlugins = config.installPlugins
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

        local args = {}

        local buildDir = System:join(self.get.projectPath, "build", "windows")
        self.get.buildDir = buildDir

        local buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")
        local clangLLVMPath = System:join(buildModule, "clang-llvm")
        self.get.clangLLVMPath = clangLLVMPath
        
        local sdl3Path = System:join(buildModule, "resources/libs/SDL3-3.2.16")

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

        if config.iconPath then
            self.get.iconPath = config.iconPath
            self.get.iconDest = System:join(buildDir, "icon.ico")
            self.get.resFile = System:join(buildDir, "icon.rc")
            self.get.resOutputFile = System:join(buildDir, "icon.res")
        end

        local compilerPath = fix_path(System:join(clangLLVMPath, "bin/clang++.exe"))
        
        -- table.insert(args, compilerPath)
        table.insert(args, fix_path(System:getRelativePath("amara2/main/main.cpp")))

        if self.get.resOutputFile then
            table.insert(args, fix_path(self.get.resOutputFile))
        end

        -- AMARA_PATH
        table.insert(args, "-Iamara2")
        if self.get.installPlugins then
            table.insert(args, "-I", fix_path(System:join(self.get.projectPath, "plugins")))
        end

        -- OTHER_LIB_PATHS
        table.insert(args, "-Isrc")
        table.insert(args, "-I" .. fix_path(nlohmannPath))
        table.insert(args, "-I" .. fix_path(luaPath))
        table.insert(args, "-I" .. fix_path(sol2Path))
        table.insert(args, "-I" .. fix_path(stbPath))
        table.insert(args, "-I" .. fix_path(glmPath))
        table.insert(args, "-I" .. fix_path(tinyxml2Path))
        table.insert(args, "-I" .. fix_path(minimp3Path))
        table.insert(args, "-I" .. fix_path(pfdPath))

        -- SDL_PATHS_WIN64
        table.insert(args, "-I" .. fix_path(System:join(sdl3Path, "include")))
        table.insert(args, "-L" .. fix_path(System:join(sdl3Path, "lib", "x64")))
        
        -- WINDOWS_COMPILER_FLAGS
        table.insert(args, "-w")
        table.insert(args, "-m64")
        table.insert(args, "-Wl,/SUBSYSTEM:WINDOWS")
        table.insert(args, "-Wl,/NOIMPLIB")
        table.insert(args, "-std=c++17")
        table.insert(args, "-O2")
        
        -- table.insert(args, "-w")
        -- table.insert(args, "-Wall")
        -- table.insert(args, "-m64")
        -- table.insert(args, "-std=c++17")
        -- table.insert(args, "-Wl,/NOIMPLIB")
        -- table.insert(args, "-DAMARA_DEBUG_BUILD")

        -- EXTRA_OPTIONS
        if self.get.installPlugins then
            table.insert(args, "-DAMARA_PLUGINS")
        end
        table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")
        -- Add flags from Makefile's EXTRA_OPTIONS
        -- table.insert(args, "-DAMARA_DEBUGGING")
        -- table.insert(args, "-DAMARA_ENGINE_TOOLS")

        if self.get.projectData.encryption then
            table.insert(args, "-DAMARA_ENCRYPTION_KEY=" .. quote_if_needed(self.get.projectData.encryption["key"]))
            if self.get.projectData.encryption["encrypt-write-output"] then
                table.insert(args, "-DAMARA_ENCRYPT_OUTPUT")
            end
        end

        -- LINKER_FLAGS_WIN64
        table.insert(args, "-fuse-ld=lld")
        table.insert(args, "-stdlib=libc++")
        table.insert(args, "-L" .. fix_path(System:join(clangLLVMPath, "lib")))
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
        table.insert(args, fix_path(System:join(buildDir, self.get.executableName .. ".exe")))

        local argsFile = System:join(buildDir, "build_args.txt")
        System:writeFile(argsFile, string.sep_concat(" ", table.unpack(args)))
        local buildCommand = quote_if_needed(compilerPath) .. " @" .. quote_if_needed(argsFile)
        
        local batchFilePath = System:join(buildDir, "build_windows.bat")
        self.get.batchFilePath = batchFilePath
        local batchFileContent = buildCommand .. " && exit"

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
            System:WriteICO(self.get.iconPath, self.get.iconDest)
            System:writeFile(self.get.resFile, "1 ICON \"" .. self.get.iconDest .. "\"\n")
            
            local command = string.format("%s \"%s\"", System:join(self.get.clangLLVMPath, "bin/llvm-rc"), self.get.resFile)
            System:execute(command)
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
                        
                        System:remove(System:join(self.get.projectPath, "build", "windows"))
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

    -- onOutput = function(self, msg)
    --     if self.get.printLog then
    --         self.get.printLog.func:handleMessage(msg)
    --     end
    -- end,

    onExit = function(self, exitCode)
        System:remove(self.get.batchFilePath)
        System:remove(System:join(self.get.buildDir, "build_args.txt"))
        
        if self.get.printLog then
            self.get.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        if exitCode == 0 then
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
            self.get.printLog.func:stopLoading()
            -- System:remove(System:join(self.get.projectPath, "build", "windows"))
            
            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))
            if not System:VSBuildToolsInstalled() then
                self.get.printLog.func:handleMessage(Localize:get("error_vsBuildToolsNotFound"))
            end
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})