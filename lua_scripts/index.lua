version_major = 0
version_minor = 2
version_patch = 14

version_string = version_major .. "." .. version_minor .. "." .. version_patch

Scripts:run("utility/Localize")
Scripts:run("utility/OpenCodeEditor")

Scripts:include("ui/UIWindow")
Scripts:include("ui/UIButton")
Scripts:include("ui/ToolTips")
Scripts:include("ui/TextField")
Scripts:include("ui/CodeEditorButton")
Scripts:include("ui/DropDownMenu")
Scripts:include("ui/LoadingBar")
Scripts:include("ui/PagedWindow")

Scripts:include("windows/InitialSetupWindow")
Scripts:include("windows/MainWindow")
Scripts:include("windows/NewProjectWindow")
Scripts:include("windows/ProjectWindow")
Scripts:include("windows/TerminalWindow")
Scripts:include("windows/ProjectSettingsWindow")
Scripts:include("windows/ExamplesWindow")
Scripts:include("windows/RunArgumentsWindow")

Scripts:include("building/BuildPlatformMenu")
Scripts:include("building/IncludeFolders")
Scripts:include("building/EncryptionOptions")

Scripts:include("building/windows/VSBuildToolsInstaller")
Scripts:include("building/windows/WindowsBuildInstaller")
Scripts:include("building/windows/WindowsBuildOptions")
Scripts:include("building/windows/WindowsBuildNode")

Scripts:include("building/web/WebBuildOptions")
Scripts:include("building/web/WebBuildNode")

Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        transparent = true,
        clickThrough = true,
        alwaysOnTop = true,
        vsync = true,
        graphics = Graphics.Render2D,
        screenMode = ScreenMode.Fullscreen,
    },
    
    onPreload = function(world)
        world:restoreWindow()

        world.load:shaderProgram("boxBlur", {
            vertex = "defaultVertex",
            fragment = "shaders/boxBlur.frag"
        })
        
        world.load:image("uiBox", "ui/amara2_uiBox.png")
        world.load:spritesheet("terminalWindow", "ui/amara2_terminalWindow.png", 32, 32)
        world.load:image("toolTipBox", "ui/amara2_toolTipBox.png")
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16)
        world.load:spritesheet("uiIcons", "ui/amara2_icons.png", 16, 16)
        world.load:spritesheet("tickBox", "ui/amara2_tickBox.png", 9, 9)
        world.load:spritesheet("largeIcons", "ui/amara2_large_icons.png", 56, 56)
        
        world.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        Localize:registerJSON(System:readJSON("files/localization/keywords.json"))
        world.windowTitle = Localize:get("title_windowTitle")
    end,

    onCreate = function(world)
        world:fitToDisplay()
        math.randomseed(os.time())
        
        local props = world.props;
        
        props.windowShadows = world:createChild("ShaderLayer", {
            alpha = 0.75,
            tint = Colors.Black
        })

        props.windows = world:createChild("ShaderLayer", {
            closeAll = function(self, _onEnd)
                for _, child in ipairs(self.children) do
                    if child.func.closeWindow then
                        child.func:closeWindow(_onEnd)
                    end
                end
            end
        })

        if System:exists("files/settings.json") and System:readJSON("files/settings.json") ~= nil then
            world.func:fixSettings()
            local win = props.windows:createChild("MainWindow")
            win.func:openWindow()
        else
            local win = props.windows:createChild("InitialSetupWindow")
            win.func:openWindow()
        end
        
        props.windowShadows_copy = props.windowShadows:createChild("CopyNode", {
            target = props.windows,
            x = -2, y = 2
        })

        props.toolTips = world:createChild("ToolTips")

        local hotkey
        hotkey = world:createChild("Hotkey", {
            config = {
                { Key.LeftCtrl, Key.LeftAlt },
                { Key.RightCtrl, Key.RightAlt }
            },
            onPress = function()
                if not world.forcedClickThrough then
                   world.clickThrough = false
                end
            end,
            onRelease = function()
                world.clickThrough = true
            end,
            whilePressed = function()
                local switchDisplay = 0
                if hotkey:pressing(Key.One) then
                    switchDisplay = 1
                elseif hotkey:pressing(Key.Two) then
                    switchDisplay = 2
                elseif hotkey:pressing(Key.Three) then
                    switchDisplay = 3
                elseif hotkey:pressing(Key.Four) then
                    switchDisplay = 4
                end

                if world.input.mouse.wheel.y < 0 then
                    switchDisplay = world.displayID - 1
                elseif world.input.mouse.wheel.y > 0 then
                    switchDisplay = world.displayID + 1
                end
                
                if world.displayID ~= switchDisplay and switchDisplay ~= 0 then
                    world:fitToDisplay(switchDisplay)
                end
            end
        })

        Game.targetFPS = 0
    end,
    
    getSettings = function(self, forceLoad)
        if forceLoad or not self.get.settings then
            if System:exists("files/settings.json") then
                self.get.settings = System:readJSON("files/settings.json")
            end
        end
        if not self.get.settings then
            self.get.settings = {}
        end
        return self.get.settings
    end,

    fixSettings = function(self)
        local settings = self.func:getSettings()

        local oldProjects = settings.projects
        if not oldProjects then
            oldProjects = {}
            settings.projects = {}
        end

        settings.projects = {}
        
        if #oldProjects > 0 then
            for i = 1, #oldProjects do
                if #settings.projects >= 4 then
                    break
                end
                if path ~= oldProjects[i] and System:exists(oldProjects[i]) and System:exists(System:join(oldProjects[i], "project.json")) then
                    table.insert(settings.projects, oldProjects[i])
                end
            end
        end

        if settings.autoOpenCodeEditor == nil then
            settings.autoOpenCodeEditor = true
        end

        self.func:saveSettings()
    end,

    saveSettings = function(self)
        System:writeFile("files/settings.json", self.func:getSettings())
    end,

    registerProject = function(self, path)
        local settings = self.func:getSettings()
        
        if not settings.projects then
            settings.projects = {}
        end

        local oldProjects = settings.projects
        settings.projects = {}
        
        if System:exists(path) then
            table.insert(settings.projects, path)
        end
        
        if #oldProjects > 0 then
            for i = 1, #oldProjects do
                if #settings.projects >= 4 then
                    break
                end
                if path ~= oldProjects[i] and System:exists(oldProjects[i]) then
                    table.insert(settings.projects, oldProjects[i])
                end
            end
        end

        self.func:saveSettings()
    end,

    loadCodeEditors = function(self, onGet)
        self:createChild("ProcessNode", {
            arguments = {
                Game.executable,
                "-context",
                System:getBasePath(),
                "-script",
                "initialSetup/ReloadCodeEditors"
            },
            onExit = function(process, exitCode, errorMessage)
                local settings = self.func:getSettings(true)
                if onGet then
                    onGet(settings.codeEditorList)
                end
            end
        })
    end,

    getCodeEditors = function(self, onGet)
        local settings = self.func:getSettings()
        if not settings.codeEditorList then
            self.func:loadCodeEditors(onGet)
        else
            if onGet then
                onGet(settings.codeEditorList)
            end
        end
    end,

    checkBuildTools = function(self)
        if Game.platform == "windows" then
            return System:VSBuildToolsInstalled()
        end
        return false
    end
})