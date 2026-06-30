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

        local build_aab = true
        self.get.build_aab = build_aab

        local projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))
        self.get.projectData = projectData

        local sdk = System:LocateAndroidSDK()

        local buildDir = System:join(self.get.projectPath, "build", "android")
        self.get.buildDir = buildDir

        local buildModule = System:getRelativePath("build_modules/amara2_windows_build_module")
        self.get.buildModule = buildModule

        local sdl_path = System:join(buildModule, "sdl_android")
        self.get.sdl_path = sdl_path

        if not sdk["bundletool"] then
            local bundletool_path = System:join(sdl_path, "bundletool", "bundletool-all-1.18.3.jar")
            if System:exists(bundletool_path) then
                sdk["bundletool"] = bundletool_path
            end
        end
        
        if System:exists(buildDir) then
            System:remove(buildDir)
        end
        System:createDirectory(buildDir)
        
        local android_package = System:join(buildDir, "android_package")
        System:createDirectory(android_package)
        self.get.android_package = android_package

        local keystore_path = System:join(self.get.projectPath, self.get.projectData["project-name"] .. ".keystore")

        local errorOutputPath = System:join(buildDir, "build_error.txt")
        self.get.errorOutputPath = errorOutputPath
        local errorCommand = " > " .. fix_path(errorOutputPath) .. " 2>&1"
        
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
            System:join(android_package, "lib/arm64-v8a", "libc++_shared.so")
        )
        System:copy(
            System:join(sdk.sysroot, "usr", "lib", "arm-linux-androideabi", "libc++_shared.so"),
            System:join(android_package, "lib/armeabi-v7a", "libc++_shared.so")
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

            table.insert(args, "-I" ..  fix_path(System:getRelativePath("amara2")))

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

        local batchFilePath = System:join(buildDir, "build_android.bat")
        self.get.batchFilePath = batchFilePath

        local app_name = self.get.projectData.android["app-name"]
        local version  = self.get.manifest_config.VERSION_NAME

        local buildCommands = {}

        -- ── Compile .so files ──────────────────────────────────────────────────
        table.insert(buildCommands,
            fix_path(sdk.compiler) .. " " .. arm64_v8a_build.argstr
        )
        table.insert(buildCommands,
            fix_path(sdk.compiler) .. " " .. arm_v7a_build.argstr
        )

        -- ── Java env ───────────────────────────────────────────────────────────
        table.insert(buildCommands, "set \"JAVA_HOME=" .. sdk.java_home .. "\"")
        table.insert(buildCommands, "set \"PATH=%JAVA_HOME%\\bin;%PATH%\"")

        -- ── d8: compile SDL jar to classes.dex ────────────────────────────────
        local sdl_jar = System:join(sdl_path, "share", "java", "SDL3", "SDL3.jar")
        table.insert(buildCommands,
            "call " .. fix_path(sdk["d8"]) ..
            " --output " .. fix_path(android_package) ..
            " --min-api 24 " .. fix_path(sdl_jar)
        )

        -- ── aapt2 compile resources ────────────────────────────────────────────
        table.insert(buildCommands,
            fix_path(sdk["aapt2"]) .. " compile --dir " ..
            fix_path(System:join(android_package, "res")) ..
            " -o " .. fix_path(System:join(android_package, "compiled_res"))
        )

        local flats = ""
        local dpis = {"mdpi", "hdpi", "xhdpi", "xxhdpi", "xxxhdpi"}
        for _, dpi in ipairs(dpis) do
            flats = flats .. " " .. fix_path(System:join(android_package, "compiled_res", "mipmap-" .. dpi .. "_ic_launcher.png.flat"))
        end

        local manifest_flags =
            " -I " .. fix_path(sdk.android_jar) ..
            " --manifest " .. fix_path(System:join(android_package, "AndroidManifest.xml")) ..
            " --min-sdk-version 24 --target-sdk-version 35" ..
            " --version-code " .. self.get.manifest_config.VERSION_CODE ..
            " --version-name " .. self.get.manifest_config.VERSION_NAME ..
            " " .. flats

        -- ── aapt2 link for APK (binary format) ────────────────────────────────
        table.insert(buildCommands,
            fix_path(sdk["aapt2"]) .. " link" ..
            " -o " .. fix_path(System:join(android_package, "base.apk")) ..
            manifest_flags
        )

        -- ── aapt2 link for AAB (proto format) ─────────────────────────────────
        if build_aab then
            table.insert(buildCommands,
                fix_path(sdk["aapt2"]) .. " link --proto-format" ..
                " -o " .. fix_path(System:join(android_package, "base_proto.apk")) ..
                manifest_flags
            )
        end

        -- ── File handling script ───────────────────────────────────────────────
        local props_json = System:join(android_package, "props.json")
        System:writeFile(props_json, self.props)
        local file_handling_args = {
            fix_path(Game.executable),
            "-context", fix_path(System:getBasePath()),
            "-script", fix_path(System:getScriptPath("building/android/AndroidFileHandling")),
            "-props_path", fix_path(props_json)
        }
        table.insert(buildCommands, table.concat(file_handling_args, " "))

        local keystore_details = self.get.projectData.android["release-keystore"]
        local release_key_alias     = keystore_details["alias"]
        local release_storepass     = keystore_details["storepass"]
        local release_keypass       = keystore_details["keypass"]
        
        -- ── Generate keystore (shared by APK + AAB) ────────────────────────────

        if not System:exists(keystore_path) then
            table.insert(buildCommands,
                fix_path(sdk["keytool"], true) ..
                " -genkeypair -keystore " .. fix_path(keystore_path, true) ..
                " -alias " .. release_key_alias ..
                " -keypass " .. release_keypass ..
                " -storepass " .. release_storepass ..
                " -dname \"CN=" .. self.get.projectData.android["package-app-name"] .. "\"" ..
                " -keyalg RSA -keysize 2048 -validity 10000"
            )
        end

        -- ════════════════════════════════════════════════════════════════════════
        -- APK pipeline
        -- ════════════════════════════════════════════════════════════════════════

        table.insert(buildCommands,
            fix_path(sdk["zipalign"]) .. " -f 4 " ..
            fix_path(System:join(android_package, "base.apk")) .. " " ..
            fix_path(System:join(android_package, "base-aligned.apk"))
        )

        table.insert(buildCommands,
            "call " .. fix_path(sdk["apksigner"]) ..
            " sign --ks " .. fix_path(keystore_path) ..
            " --ks-pass pass:" .. release_storepass ..
            " --key-pass pass:" .. release_keypass ..
            " --ks-key-alias " .. release_key_alias ..
            " --out " .. fix_path(System:join(android_package, "base-signed.apk")) ..
            " " .. fix_path(System:join(android_package, "base-aligned.apk"))
        )

        -- ════════════════════════════════════════════════════════════════════════
        -- AAB pipeline
        -- ════════════════════════════════════════════════════════════════════════
        if build_aab then
            local base_extracted  = System:join(android_package, "base_extracted")
            local base_module_dir = System:join(android_package, "base_module")
            local base_zip        = System:join(android_package, "base.zip")
            local unsigned_aab    = System:join(android_package, "unsigned.aab")
            
            local base_proto_apk = System:join(android_package, "base_proto.apk")
            local base_proto_zip = System:join(android_package, "base_proto.zip")
            
            table.insert(buildCommands,
                "copy " ..
                fix_path(base_proto_apk, true) ..
                " " ..
                fix_path(base_proto_zip, true)
            )

            table.insert(buildCommands,
                "powershell -Command \"Expand-Archive -Force -Path '" ..
                base_proto_zip ..
                "' -DestinationPath '" ..
                base_extracted ..
                "'\""
            )

            -- Create module layout dirs
            for _, dir in ipairs({
                base_module_dir .. "\\manifest",
                base_module_dir .. "\\dex",
                base_module_dir .. "\\lib\\arm64-v8a",
                base_module_dir .. "\\lib\\armeabi-v7a",
                base_module_dir .. "\\assets",
            }) do
                table.insert(buildCommands, "mkdir " .. fix_path(dir))
            end

            -- Populate module layout
            table.insert(buildCommands,
                "copy " ..
                fix_path(System:join(base_extracted, "AndroidManifest.xml"), true) ..
                " " ..
                fix_path(System:join(base_module_dir, "manifest"), true)
            )
            table.insert(buildCommands,
                "copy " ..
                fix_path(System:join(base_extracted, "resources.pb"), true) ..
                " " ..
                fix_path(base_module_dir, true)
            )
            table.insert(buildCommands,
                "copy " ..
                fix_path(System:join(android_package, "classes.dex"), true) ..
                " " ..
                fix_path(System:join(base_module_dir, "dex"), true)
            )
            table.insert(buildCommands,
                "xcopy /E /I /Y " ..
                fix_path(System:join(android_package, "lib"), true) ..
                " " ..
                fix_path(System:join(base_module_dir, "lib"), true)
            )
            table.insert(buildCommands,
                "xcopy /E /I /Y " ..
                fix_path(System:join(android_package, "assets"), true) ..
                " " ..
                fix_path(System:join(base_module_dir, "assets"), true)
            )
            table.insert(buildCommands,
                "xcopy /E /I /Y " ..
                fix_path(System:join(base_extracted, "res"), true) ..
                " " ..
                fix_path(System:join(base_module_dir, "res"), true)
            )

            -- Zip the module
            table.insert(buildCommands,
                "powershell -Command \"" ..
                "Add-Type -Assembly System.IO.Compression.FileSystem; " ..
                "$zip = [System.IO.Compression.ZipFile]::Open('" .. base_zip .. "', 'Create'); " ..
                "$base = '" .. base_module_dir .. "'; " ..
                "Get-ChildItem -Path $base -Recurse -File | ForEach-Object { " ..
                    "$rel = $_.FullName.Substring($base.Length + 1) -replace '[\\\\]', '/'; " ..
                    "[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $_.FullName, $rel) | Out-Null " ..
                "}; " ..
                "$zip.Dispose()\""
            )

            table.insert(buildCommands,
                "powershell -Command \"Add-Type -Assembly System.IO.Compression.FileSystem; " ..
                "$z = [System.IO.Compression.ZipFile]::OpenRead('" .. base_zip .. "'); " ..
                "$z.Entries | ForEach-Object { Write-Host $_.FullName }; $z.Dispose()\""
            )

            -- bundletool build-bundle
            table.insert(buildCommands,
                "java -jar " .. fix_path(sdk["bundletool"]) ..
                " build-bundle --modules=" .. fix_path(base_zip) ..
                " --output=" .. fix_path(unsigned_aab)
            )

            -- jarsigner (reuses keystore from APK step)
            local signed_aab = System:join(android_package, "signed.aab")

            table.insert(buildCommands,
                "jarsigner -keystore " .. fix_path(keystore_path, true) ..
                " -storepass " .. release_storepass ..
                " -keypass " .. release_keypass ..
                " -signedjar " .. fix_path(signed_aab, true) ..
                " " .. fix_path(unsigned_aab, true) .. " " .. release_key_alias
            )
        end

        table.insert(buildCommands, "exit")

        local buildCommand = table.concat(buildCommands, " " .. errorCommand .. "\n")
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
            PACKAGE_NAME       = "com." .. self.get.projectData.android["package-org-name"] .. "." .. self.get.projectData.android["package-app-name"],
            VERSION_CODE       = self.get.projectData.android["version-code"] or 1,
            VERSION_NAME       = self.get.projectData.android["version-name"] or "1.0.0",
            MIN_SDK_VERSION    = 28,
            TARGET_SDK_VERSION = 35,
            GLES_VERSION       = "0x00030000",
            APP_NAME           = self.get.projectData.android["app-name"],
            ICON_NAME          = "ic_launcher",
            SCREEN_ORIENTATION = self.get.projectData.android["orientation"] or "portrait",
            KEEP_SCREEN_ON     = true,
            CATEGORY_GAME      = "<category android:name=\"android.intent.category.GAME\" />",
            MAIN_LIBRARY_NAME  = "main",
            DEPTH_SIZE         = 0
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
            ["mipmap-mdpi"]    = 48,
            ["mipmap-hdpi"]    = 72,
            ["mipmap-xhdpi"]   = 96,
            ["mipmap-xxhdpi"]  = 144,
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
            Assets:resizeTextureToPNG("app_icon", size, size, path)
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
        if self.get.printLog then
            self.get.printLog.func:unbindGameProcess()
        end

        self.world.forcedClickThrough = true
        self.world:hideWindow()

        local buildDir    = self.get.buildDir
        local app_name    = self.get.projectData.android["app-name"]
        local version     = self.get.manifest_config.VERSION_NAME
        local out_dir     = System:join(self.get.projectPath, "build", "android")

        if exitCode == 0 then
            if System:exists(self.get.errorOutputPath) then
                System:remove(self.get.errorOutputPath)
            end

            -- Copy APK
            System:copy(
                System:join(self.get.android_package, "base-signed.apk"),
                System:join(out_dir, app_name .. " " .. version .. ".apk")
            )

            -- Copy AAB
            if self.get.build_aab then
                System:copy(
                    System:join(self.get.android_package, "signed.aab"),
                    System:join(out_dir, app_name .. " " .. version .. ".aab")
                )
            end

            System:openDirectory(out_dir)
            self.get.printLog.func:handleMessage(Localize:get("label_buildSuccess"))
            self.get.printLog.func:unbindGameProcess()
            self.get.printLog.func:stopLoading()
        else
            self.get.printLog.func:stopLoading()

            if System:exists(self.get.errorOutputPath) then
                local error_message = System:readFile(self.get.errorOutputPath)
                self.get.printLog.func:handleMessage("Error: Build failed.\n" .. error_message)
                System:remove(self.get.errorOutputPath)
            end
            
            self.get.printLog.func:handleMessage(Localize:get("label_buildFailed"))
        end

        System:remove(self.get.batchFilePath)
        for _, build in ipairs(self.get.builds) do
            System:remove(build.file)
        end
        System:remove(self.get.android_package)

        self.world.forcedClickThrough = false
        self.world:showWindow()
    end
})