Nodes:define("MainWindow", "UIWindow", {
    width = 256,
    height = 156,

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local settings = self.world.func:getSettings()

        local title = self.get.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_windowTitle") .. " v" .. version_string,
            color = "#a8bee0",
            origin = 0
        })

        local recentProjectsTitle = self.get.content:createChild("Text", {
            x = 10, y = 22,
            font = "defaultFont",
            text = Localize:get("title_recentProjects"),
            color = "#f0f6ff",
            origin = 0
        })

       local backer = self.get.content:createChild("FillRect", {
            x = 6, y = recentProjectsTitle.y + 16,
            width = self.get.targetWidth - 12,
            height = 72,
            color = "#111d27",
            origin = 0
        })

        local settings = self.world.func:getSettings()

        local spacing = 15

        local failedProjects = 0

        self.get.projectButtons = {}

        if settings.projects and #settings.projects > 0 then
            local window = self

            for i = 1, #settings.projects do
                local projectPath = settings.projects[i]
                if System:exists(projectPath) and System:exists(System:join(projectPath, "project.json")) then
                    local projectData = System:readJSON(System:join(projectPath, "project.json"))
                    local projectName = projectData["project-name"]

                    local optBacker = self.get.content:createChild("FillRect", {
                        x = backer.x + 6,
                        y = backer.y + 6 + (i - 1 - failedProjects) * spacing,
                        width = backer.width - 12,
                        height = spacing - 2,
                        color = "#111d27",
                        origin = 0,
                        props = {
                            projectPath = projectPath
                        },
                        input = {
                            active = true,
                            cursor = Cursor.Pointer,
                            onPointerHover = function(self, pointer)
                                self.color = "#333e4d"
                            end,
                            onPointerExit = function(self, pointer)
                                self.color = "#111d27"
                            end,
                            onPointerUp = function(self, pointer)
                                self.func:forcePress()
                            end
                        },
                        forcePress = function(self)
                            self.input:deactivate()
                            window.func:closeWindow(function()
                                local newWindow = window.parent:createChild("ProjectWindow", {
                                    projectPath = self.get.projectPath
                                })
                                newWindow.func:openWindow()
                                newWindow.func:openDefault()
                                
                                window:destroy()
                            end)
                        end
                    })
                    self.get.projectButtons[i] = optBacker

                    local txt = self.get.content:createChild("Text", {
                        text = projectName,
                        font = "defaultFont",
                        color = "#a8bee0",
                        origin = 0,
                        x = optBacker.x + 4,
                        y = optBacker.y
                    })
                    local str = projectName
                    while txt.width > optBacker.width - 8 do
                        str = string.sub(str, 2)
                        txt.text = string.concat("...", str)
                    end
                else
                    failedProjects = failedProjects + 1
                end
            end
        end

        local editorTitle = self.get.content:createChild("Text", {
            x = recentProjectsTitle.x,
            y = backer.y + backer.height + 6,
            origin = 0,
            font = "defaultFont",
            color = Colors.White,
            text = Localize:get("title_codeEditor")
        })

        local editorMenu = self.get.content:createChild("DropDownMenu", {
            x = backer.x,
            y = editorTitle.y + editorTitle.height + 6,
            width = backer.width,
            defaultText = Localize:get("label_noCodeEditorAvailable"),
            onSelect = function(menu, opt)
                local settings = self.world.func:getSettings()
                settings.codeEditor = opt
                self.world.func:saveSettings()
            end
        })
        self.get.editorMenu = editorMenu
        self.func:loadCodeEditors()
        
        local refreshButton = self.get.content:createChild("UIButton", {
            id = "refreshEditorsButton",
            toolTip = "toolTip_refreshCodeEditors",
            x = editorTitle.x + editorTitle.width + 4,
            icon = 18,
            onPress = function()
                local settings = self.world.func:getSettings()
                settings.codeEditorList = nil
                self.func:loadCodeEditors()
            end
        })
        refreshButton.y = editorMenu.y - refreshButton.height - 2

        local tickBox
        tickBox = self.get.content:createChild("Sprite", {
            origin = { 1, 0 },
            x = backer.x + backer.width,
            y = editorTitle.y + 2,
            frame = settings.autoOpenCodeEditor and 2 or 1,
            texture = "tickBox",
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function()
                    local settings = self.world.func:getSettings()
                    settings.autoOpenCodeEditor = not settings.autoOpenCodeEditor
                    tickBox.frame = settings.autoOpenCodeEditor and 2 or 1
                    self.world.func:saveSettings()
                end
            }
        })

        local autoOpenTitle = self.get.content:createChild("Text", {
            x = tickBox.x - tickBox.width - 2,
            y = editorTitle.y,
            origin = { 1, 0 },
            font = "defaultFont",
            color = Colors.White,
            text = Localize:get("label_autoOpenCodeEditor"),
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function()
                    local settings = self.world.func:getSettings()
                    settings.autoOpenCodeEditor = not settings.autoOpenCodeEditor
                    tickBox.frame = settings.autoOpenCodeEditor and 2 or 1
                    self.world.func:saveSettings()
                end
            }
        })

        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        self.get.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 1,
            onPress = function()
                self.world.get.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                self.get.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "minimizeButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function(self)
                self.world:minimizeWindow()
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Minus },
                { Key.RightAlt, Key.RightShift, Key.Minus }
            }
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "openDocsButton",
            toolTip = "toolTip_openDocs",
            x = buttonPos,
            y = 4,
            icon = 19,
            onPress = function()
                System:openWebsite("https://github.com/BigBossErndog/Amara2/wiki")
            end
        })

        buttonPos = buttonPos - buttonSpacing
        local examplesButton = self.get.content:createChild("UIButton", {
            id = "openExamplesButton",
            toolTip = "toolTip_openExamples",
            x = buttonPos,
            y = 4,
            icon = 20,
            onPress = function(button)
                
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openExistingProject",
            x = buttonPos,
            y = 4,
            icon = 6,
            onPress = function(button)
                self.get.enabled = false
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    local path = nil

                    if settings.projects and #settings.projects > 0 then
                        if System:exists(System:getDirectoryOf(settings.projects[1])) then
                            path = System:browseDirectory(System:getDirectoryOf(settings.projects[1]))
                        end
                    end
                    
                    if not path then
                        path = System:browseDirectory()
                    end

                    self.world:showWindow()

                    if string.len(path) == 0 then
                        return
                    end

                    local indexPath = System:join(path, "lua_scripts", "index.lua")
                    if not System:exists(indexPath) then
                        return
                    end

                    local projectJsonPath = System:join(path, "project.json")
                    if not System:exists(projectJsonPath) then
                        return
                    end

                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = path
                    })
                    newWindow.func:openDefault()
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.O },
                { Key.RightAlt, Key.RightShift, Key.O }
            }
        })

        buttonPos = buttonPos - buttonSpacing
        self.get.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_newProject",
            x = buttonPos,
            y = 4,
            icon = 2,
            onPress = function()
                self.func:closeWindow(function()
                    self.get.enabled = false

                    local newWindow = self.parent:createChild("NewProjectWindow", {
                        x = self.x, y = self.y
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.N },
                { Key.RightAlt, Key.RightShift, Key.N }
            }
        })

        if not settings.projects or failedProjects >= #settings.projects then
            self.get.content:createChild("Text", {
                text = Localize:get("label_noProjectsFound"),
                font = "defaultFont",
                color = "#a8bee0",
                origin = 0,
                x = backer.x + 6 + 4,
                y = backer.y + 6
            })
        end

        self.func:setHotkeys()
    end,

    setHotkeys = function(self)
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.One },
                { Key.RightAlt, Key.RightShift, Key.One }
            },
            onPress = function()
                if self.get.projectButtons[1] then
                    self.get.projectButtons[1].func:forcePress()
                end
            end
        })
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Two },
                { Key.RightAlt, Key.RightShift, Key.Two }
            },
            onPress = function()
                if self.get.projectButtons[2] then
                    self.get.projectButtons[2].func:forcePress()
                end
            end
        })
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Three },
                { Key.RightAlt, Key.RightShift, Key.Three }
            },
            onPress = function()
                if self.get.projectButtons[3] then
                    self.get.projectButtons[3].func:forcePress()
                end
            end
        })
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Four },
                { Key.RightAlt, Key.RightShift, Key.Four }
            },
            onPress = function()
                if self.get.projectButtons[4] then
                    self.get.projectButtons[4].func:forcePress()
                end
            end
        })
    end,

    loadCodeEditors = function(self)
        self.world.func:getCodeEditors(function(codeEditors)
            if not self.destroyed then
                self.get.editorMenu.func:createOptions(codeEditors)

                if codeEditors and #codeEditors > 0 then
                    local settings = self.world.func:getSettings()
                    if settings.codeEditor then
                        local found = false
                        for i = 1, #codeEditors do
                            if codeEditors[i] == settings.codeEditor then
                                found = true
                                self.get.editorMenu.func:select(codeEditors[i])
                                break
                            end
                        end
                        if not found then
                            self.get.editorMenu.func:select(codeEditors[1])
                        end
                    else
                        self.get.editorMenu.func:select(codeEditors[1])
                    end
                end
            end
        end)
    end
})