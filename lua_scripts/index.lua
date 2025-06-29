Scripts:run("utility/localize")

Nodes:load("UIWindow", "ui/UIWindow")
Nodes:load("UIButton", "ui/UIButton")
Nodes:load("ToolTips", "ui/ToolTips")
Nodes:load("TextField", "ui/TextField")
Nodes:load("CodeEditorButton", "ui/CodeEditorButton")
Nodes:load("DropDownMenu", "ui/DropDownMenu")

Nodes:load("MainWindow", "windows/MainWindow")
Nodes:load("NewProjectWindow", "windows/NewProjectWindow")
Nodes:load("ProjectWindow", "windows/ProjectWindow")
Nodes:load("TerminalWindow", "windows/TerminalWindow")
Nodes:load("CopyProjectWindow", "windows/CopyProjectWindow")
Nodes:load("BuildOptions", "windows/BuildOptions")

Nodes:load("BuildNode", "utility/BuildNode")

return Creator:createWorld({
    window = {
        width = 1280,
        height = 720,
        virtualWidth = 640,
        virtualHeight = 360,
        screenMode = ScreenMode.BorderlessFullscreen,
        transparent = true,
        clickThrough = true,
        alwaysOnTop = true,
        vsync = true,
        graphics = Graphics.Render2D
    },
    
    onPreload = function(world) 
        world:fitToDisplay()
        world:restoreWindow()
        
        world.load:image("uiBox", "ui/amara2_uiBox.png")
        world.load:spritesheet("terminalWindow", "ui/amara2_terminalWindow.png", 32, 32)
        world.load:image("toolTipBox", "ui/amara2_toolTipBox.png")
        world.load:spritesheet("uiButton", "ui/amara2_uiButton.png", 16, 16)
        world.load:spritesheet("uiIcons", "ui/amara2_icons.png", 16, 16)
        world.load:spritesheet("tickBox", "ui/amara2_tickBox.png", 9, 9)
        
        world.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        Localize:registerJSON(System:readJSON("data/localization/keywords.json"))
        world.windowTitle = Localize:get("title_windowTitle")

        world.func:loadCodeEditors()
    end,

    onCreate = function(world)
        local props = world.props;

        world.func:fixSettings()

        world.func:checkBuildTools()

        props.windowShadows = world:createChild("TextureContainer", {
            alpha = 0.5,
            tint = Colors.Black,
            onCreate = function(self)
                self.size = self.world.view
            end,
            onUpdate = function(self)
                self.size = self.world.view
            end
        })

        props.windows = world:createChild("Group", {
            closeAll = function(self, _onEnd)
                for _, child in ipairs(self.children) do
                    if child.func.closeWindow then
                        child.func:closeWindow(_onEnd)
                    end
                end
            end
        })

        props.mainwin = props.windows:createChild("MainWindow")
        props.mainwin.func:openWindow()

        props.windowShadows_copy = props.windowShadows:createChild("CopyNode", {
            target = props.windows,
            x = -6, y = 8
        })

        props.toolTips = world:createChild("ToolTips")
    end,

    onUpdate = function(world, deltaTime)
        if Keyboard:isDown(Key.LeftCtrl) and Keyboard:isDown(Key.LeftAlt) then
            if Keyboard:justPressed(Key.One) then
                world:fitToDisplay(1)
            end
            if Keyboard:justPressed(Key.Two) then
                world:fitToDisplay(2)
            end
            if Keyboard:justPressed(Key.Three) then
                world:fitToDisplay(3)
            end
            if Keyboard:justPressed(Key.Four) then
                world:fitToDisplay(4)
            end
        end
    end,

    getSettings = function(self)
        if not self.props.settings then
            if System:exists("data/settings.json") then
                self.props.settings = System:readJSON("data/settings.json")
            else
                self.props.settings = {}
            end
        end

        return self.props.settings
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
        System:writeFile("data/settings.json", self.func:getSettings())
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

    loadCodeEditors = function(self)
        local settings = self.func:getSettings()

        local editors = {}

        if System:programInstalled("code") then
            table.insert(editors, "codeEditor_VSCode")
        end
        if System:programInstalled("code-insiders") then
            table.insert(editors, "codeEditor_VSCodeInsiders")
        end
        if System:programInstalled("code-oss") then
            table.insert(editors, "codeEditor_CodeOSS")
        end
        if System:programInstalled("atom") then
            table.insert(editors, "codeEditor_Atom")
        end
        if System:programInstalled("sublime_text") then
            table.insert(editors, "codeEditor_Sublime-Text")
        end
        if System:programInstalled("subl") then
            table.insert(editors, "codeEditor_Sublime")
        end
        if System:programInstalled("notepad") then
            table.insert(editors, "codeEditor_Notepad")
        end

        if #editors > 0 then
            if not settings.codeEditor then
                settings.codeEditor = editors[1]
            end
        end

        if #editors > 0 then
            self.props.editors = editors
        else
            self.props.editors = nil
        end


        self.func:saveSettings()
    end,

    checkBuildTools = function(self)
        if Game.platform == "windows" then
            return System:VSBuildToolsInstalled()
        end
        return false
    end
})