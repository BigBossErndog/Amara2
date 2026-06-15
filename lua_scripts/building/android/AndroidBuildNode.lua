Nodes:define("AndroidBuildNode", "ProcessNode", {
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

        local function fix_path(path, allow_backslashes)
            if not allow_backslashes then
                return quote_if_needed(string.gsub(path, "\\", "/"))
            else
                return quote_if_needed(path)
            end
        end

        local function escape_double_quotes(str)
            return (str:gsub('"', '\\"'))
        end

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        self.get.projectData = projectData

        local sdk = System:LocateAndroidSDK()

        local buildDir = System:join(self.get.projectPath, "build", "android")
        self.get.buildDir = buildDir

        local buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")
        self.get.buildModule = buildModule

        local sdl_path = System:join(buildModule, "sdl_android")
        self.get.sdl_path = sdl_path

        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)
        
        local android_package = System:join(buildDir, "android_package")
        System:createDirectory(android_package)
        self.get.android_package = android_package

        local setup_folders = {
            "lib/arm64-v8a",
            "lib/armeabi-v7a",
            "assets",
            "res",
            "compiled_res"
        }
        for i = 1, #setup_folders do
            System:createDirectory(System:join(android_package, setup_folders[i]))
        end

        System:copy(
            System:join(sdl_path, "lib", "arm64-v8a", "libSDL3.so"),
            System:join(android_package, "lib/arm64-v8a", "libSDL3.so")
        )
        System:copy(
            System:join(sdl_path, "lib", "armeabi-v7a", "libSDL3.so"),
            System:join(android_package, "lib/armeabi-v7a", "libSDL3.so")
        )

        System:copy(
            System:join(sdk.sysroot, "usr", "lib", "aarch64-linux-android", "libc++_shared.so"),
            System:join(self.get.android_package, "lib/arm64-v8a", "libc++_shared.so")
        )
        System:copy(
            System:join(sdk.sysroot, "usr", "lib", "arm-linux-androideabi", "libc++_shared.so"),
            System:join(self.get.android_package, "lib/armeabi-v7a", "libc++_shared.so")
        )

        self.func:generateManifest()
        self.func:generateIcons()

        if config.printLog then
            self.get.printLog = config.printLog
        end

        local buildArgs = function(target, targetCommand)
            local args = {}

            if targetCommand then
                table.insert(args, targetCommand)
            end

            table.insert(args, "-shared")
            table.insert(args, "-o")
            table.insert(args, fix_path(System:join(android_package, "lib", target, "libmain.so")))
            table.insert(args, fix_path(System:getRelativePath("amara2/main/main.cpp")))

            table.insert(args, "-w")
            table.insert(args, "-std=c++17")

            table.insert(args, "-Iamara2")

            local static_libs = {}
            
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

            table.insert(args, "--sysroot=" .. fix_path(sdk.sysroot))
            table.insert(args, "-I" .. fix_path(System:join(sdl_path, "include")))
            table.insert(args, "-L" .. fix_path(System:join(android_package, "lib", target)))
            table.insert(args, "-lSDL3")
            table.insert(args, "-landroid")
            table.insert(args, "-llog")
            table.insert(args, "-lGLESv3")
            table.insert(args, "-fPIC")
            table.insert(args, "-ftls-model=global-dynamic")

            if self.get.projectData["plugin-directories"] and #self.get.projectData["plugin-directories"] > 0 then
                table.insert(args, "-DAMARA_PLUGINS")
            end
            table.insert(args, "-DAMARA_DISABLE_EXTERNAL_SCRIPTS")
            table.insert(args, "-DAMARA_DEF_ORG=\\\"" .. self.get.projectData.android["package-org-name"] .. "\\\"")
            table.insert(args, "-DAMARA_DEF_APP=\\\"" .. self.get.projectData.android["package-app-name"] .. "\\\"")
            
            if self.get.projectData.encryption and not config.buildTest then
                table.insert(args, "-DAMARA_ENCRYPTION_KEY=" .. quote_if_needed(self.get.projectData.encryption["key"]))
                if self.get.projectData.encryption["encrypt-write-output"] then
                    table.insert(args, "-DAMARA_ENCRYPT_OUTPUT")
                end
            end

            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/json/include")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/lua")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/sol2")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/stb")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/glm")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/minimp3")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/portable-file-dialogs")))
            table.insert(args, "-I" .. fix_path(System:join(buildModule, "resources/libs/tinyxml2")))

            return args
        end

        local arm64_v8a_build = {
            args = buildArgs("arm64-v8a", "-target aarch64-linux-android33"),
            file = System:join(buildDir, "arm64-v8a_build.txt")
        }
        arm64_v8a_build.argstr = string.sep_concat(" ", table.unpack(arm64_v8a_build.args))
        local arm_v7a_build = {
            args = buildArgs("armeabi-v7a", "-target armv7a-linux-androideabi33"),
            file = System:join(buildDir, "armeabi-v7a_build.txt")
        }
        arm_v7a_build.argstr = string.sep_concat(" ", table.unpack(arm_v7a_build.args))
        
        local builds = {
            arm64_v8a_build,
            arm_v7a_build
        }
        for _, build in ipairs(builds) do
            System:writeFile(build.file, build.argstr)
        end
        self.get.builds = builds

        local errorOutputPath = System:join(buildDir, "build_error.txt")
        self.get.errorOutputPath = errorOutputPath
        
        local batchFilePath = System:join(buildDir, "build_android.bat")
        self.get.batchFilePath = batchFilePath

        local buildCommands = {
            fix_path(sdk.compiler) .. " " .. arm64_v8a_build.argstr .. " > " .. fix_path(errorOutputPath) .. " 2>&1",
            fix_path(sdk.compiler) .. " " .. arm_v7a_build.argstr .. " > " .. fix_path(errorOutputPath) .. " 2>&1"
        }
        
        local sdl_jar = System:join(sdl_path, "share", "java", "SDL3", "SDL3.jar")
        table.insert(buildCommands,
            "set \"JAVA_HOME=" .. sdk.java_home .. "\""
        )
        table.insert(buildCommands,
            "set \"PATH=%JAVA_HOME%\\bin;%PATH%\""
        )

        table.insert(buildCommands,
            "call " .. fix_path(sdk["d8"]) .. " --output " .. fix_path(self.get.android_package) .. " --min-api 24 " .. fix_path(sdl_jar) .. " > " .. fix_path(errorOutputPath) .. " 2>&1"
        )

        table.insert(buildCommands,
            fix_path(sdk["aapt2"]) .. " compile --dir " .. fix_path(System:join(self.get.android_package, "res")) .. " -o " .. fix_path(System:join(self.get.android_package, "compiled_res"))
        )
        
        local flats = ""
        local abis = {"mdpi", "hdpi", "xhdpi", "xxhdpi", "xxxhdpi"}
        for _, dpi in ipairs(abis) do
            flats = flats .. " " .. fix_path(System:join(self.get.android_package, "compiled_res", "mipmap-" .. dpi .. "_ic_launcher.png.flat"))
        end
        table.insert(buildCommands,
            fix_path(sdk["aapt2"]) .. " link -o " .. fix_path(System:join(self.get.android_package, "base.apk")) .. " -I " .. fix_path(sdk.android_jar) .. " --manifest " .. fix_path(System:join(self.get.android_package, "AndroidManifest.xml")) .. " --min-sdk-version 24 --target-sdk-version 35 --version-code " .. self.get.manifest_config.VERSION_CODE .. " --version-name " .. self.get.manifest_config.VERSION_NAME .. " " .. flats
        )

        local props_json = System:join(self.get.android_package, "props.json")
        System:writeFile(props_json, self.props)
        local file_handling_args = {
            fix_path(Game.executable),
            "-context", fix_path(System:getBasePath()),
            "-script", fix_path(System:getScriptPath("building/android/AndroidFileHandling")),
            "-props_path", fix_path(props_json)
        }
        table.insert(buildCommands,
            table.concat(file_handling_args, " ")
        )
        
        table.insert(buildCommands,
            fix_path(sdk["zipalign"]) .. " -f 4 " .. fix_path(System:join(self.get.android_package, "base.apk")) .. " " .. fix_path(System:join(self.get.android_package, "base-aligned.apk"))
        )

        local keystore_path = System:join(self.get.android_package, "debug.keystore")
        if System:exists(keystore_path) then
            System:remove(keystore_path)
        end
        table.insert(buildCommands,
            fix_path(sdk["keytool"]) .. " -genkeypair -keystore " .. fix_path(keystore_path) .. " -alias androiddebugkey -keypass android -storepass android -dname \"CN=Android Debug,O=Android,C=US\" -keyalg RSA -keysize 2048 -validity 10000"
        )

        table.insert(buildCommands,
            "call " .. fix_path(sdk["apksigner"]) .. " sign --ks " .. fix_path(keystore_path) .. " --ks-pass pass:android --key-pass pass:android --ks-key-alias androiddebugkey --out " .. fix_path(System:join(self.get.android_package, "base-signed.apk")) .. " " .. fix_path(System:join(self.get.android_package, "base-aligned.apk"))
        )

        table.insert(buildCommands, "exit")

        local buildCommand = table.concat(buildCommands, "\n")
        System:writeFile(batchFilePath, buildCommand)

        local systemCommand = "System:exit(System:executeTerminal(" .. string.format("%q", quote_if_needed(batchFilePath)) .. "))"

        self:configure({
            arguments = {
                Game.executable,
                "-context", System:getBasePath(),
                "-inline-script", systemCommand,
                "-inline-override"
            }
        })
    end,

    generateManifest = function(self)
        local template = System:readFile(System:join(self.get.sdl_path, "AndroidManifest.xml"))
        
        local config = {
            PACKAGE_NAME       = self.get.projectData.android["package-org-name"] .. "." .. self.get.projectData.android["package-app-name"],
            VERSION_CODE       = self.get.projectData["version-code"] or 1,
            VERSION_NAME       = self.get.projectData["version-name"] or "1.0.0",
            MIN_SDK_VERSION    = 24,
            TARGET_SDK_VERSION = 35,
            GLES_VERSION       = "0x00030000",
            APP_NAME           = self.get.projectData.android["app-name"],
            ICON_NAME          = "ic_launcher",
            SCREEN_ORIENTATION = self.get.projectData.android["orientation"] or "portrait",
            KEEP_SCREEN_ON = true,
            CATEGORY_GAME = "<category android:name=\"android.intent.category.GAME\" />",
            MAIN_LIBRARY_NAME = "main",
            DEPTH_SIZE = 0
        }
        self.get.manifest_config = config

        local lines = {}
        for line in template:gmatch("[^\n]+") do
            local missing = false
            for key in line:gmatch("{(%u[%u_]*)}") do
                if config[key] == nil then
                    missing = true
                    break
                end
            end

            if not missing then
                local replaced = line:gsub("{(%u[%u_]*)}", function(key)
                    return tostring(config[key])
                end)
                lines[#lines + 1] = replaced
            end
        end

        local manifest = table.concat(lines, "\n")
        System:writeFile(System:join(self.get.android_package, "AndroidManifest.xml"), manifest)
    end,

    generateIcons = function(self)
        local mimaps = {
            ["mipmap-mdpi"] = 48,
            ["mipmap-hdpi"] = 72,
            ["mipmap-xhdpi"] = 96,
            ["mipmap-xxhdpi"] = 144,
            ["mipmap-xxxhdpi"] = 192
        }

        local icon_path = self.get.projectData.android["app-icon"]
        if not icon_path then
            local defaultIcon = System:getRelativePath("assets/icons/icon.png")
            if not System:exists(defaultIcon) then
                return
            end
            icon_path = defaultIcon
        end

        self.load:image("app_icon", icon_path)
        for k, size in pairs(mimaps) do
            local path = System:join(self.get.android_package, "res", k, "ic_launcher.png")
            Assets:resizeTextureToPNG(
                "app_icon",
                size, size,
                path
            )
        end
    end,

    onPrepare = function(actor)
        local self = actor:getChild("buildNode")

        self.world:hideWindow()

        if not self.get.printLog then
            self.get.printLog = self.world.get.windows:createChild("TerminalWindow", {
                titleText = "title_androidBuilder",
                gameProcess = self,
                props = {
                    projectPath = self.get.projectPath
                },
                allowMinimize = true,
                disableSavePosition = true,
                onExit = function(self)
                    if self.get.gameProcess then
                        System:remove(self.get.gameProcess.get.batchFilePath)
                        for _, build in ipairs(self.get.gameProcess.get.builds) do
                            System:remove(build.file)
                        end
                    end

                    local newWindow = self.world.get.windows:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath
                    })
                    
                    if self.get.gameProcess then
                        self.get.gameProcess:destroy()
                        self.get.gameProcess = nil
                        
                        System:remove(System:join(self.get.projectPath, "build", "android"))
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
        for _, build in ipairs(self.get.builds) do
            System:remove(build.file)
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

            System:copy(
                System:join(self.get.android_package, "base-signed.apk"),
                System:join(self.get.projectPath, "build", "android", self.get.projectData.android["app-name"] .. " " .. self.get.manifest_config.VERSION_NAME .. ".apk")
            )
            System:remove(self.get.android_package)

            System:openDirectory(System:join(self.get.projectPath, "build", "android"))
            self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
            self.get.printLog.func:unbindGameProcess()
            self.get.printLog.func:stopLoading()
        else
            self.get.printLog.func:stopLoading()

            local error_message = nil
            if System:exists(errorOutputPath) then
                error_message = System:readFile(errorOutputPath)
                self.get.printLog.func:handleMessage("Error: Build failed.\n" .. error_message)
                System:remove(errorOutputPath)
            end
            
            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))
        end

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})