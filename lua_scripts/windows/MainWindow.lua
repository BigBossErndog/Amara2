return Nodes:define("MainWindow", "UIWindow", {
    width = 256,
    height = 140,
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local settings = self.world.func:getSettings()

        local title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_recentProjects"),
            color = "#f0f6ff",
            origin = 0
        })

       local backer = self.props.content:createChild("FillRect", {
            x = 6, y = 24,
            width = self.props.targetWidth - 12,
            height = 72,
            color = "#111d27",
            origin = 0
        })

        local settings = self.world.func:getSettings()

        local spacing = 15

        local failedProjects = 0

        if settings.projects and #settings.projects > 0 then
            for i = 1, #settings.projects do
                local projectPath = settings.projects[i]
                if System:exists(projectPath) and System:exists(System:join(projectPath, "project.json")) then
                    local projectData = System:readJSON(System:join(projectPath, "project.json"))
                    local projectName = projectData["project-name"]
                    
                    local window = self

                    local optBacker = self.props.content:createChild("FillRect", {
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
                                self.input:deactivate()
                                window.func:closeWindow(function()
                                    local newWindow = window.parent:createChild("ProjectWindow", {
                                        projectPath = self.props.projectPath
                                    })
                                    newWindow.func:openWindow()
                                    newWindow.func:openDefault()
                                    
                                    window:destroy()
                                end)
                            end
                        }
                    })

                    local txt = self.props.content:createChild("Text", {
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

        local editorTitle = self.props.content:createChild("Text", {
            x = title.x,
            y = backer.y + backer.height + 6,
            origin = 0,
            font = "defaultFont",
            color = Colors.White,
            text = Localize:get("title_codeEditor")
        })

        local editorMenu = self.props.content:createChild("DropDownMenu", {
            x = backer.x,
            y = editorTitle.y + editorTitle.height + 6,
            width = backer.width,
            defaultText = Localize:get("label_noCodeEditorAvailable"),
            onSelect = function(menu, opt)
                settings.codeEditor = opt
                self.world.func:saveSettings()
            end
        })
        self.props.editorMenu = editorMenu
        self.func:loadCodeEditors()
        
        local refreshButton = self.props.content:createChild("UIButton", {
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
        tickBox = self.props.content:createChild("Sprite", {
            origin = { 1, 0 },
            x = backer.x + backer.width,
            y = editorTitle.y + 2,
            frame = settings.autoOpenCodeEditor and 2 or 1,
            texture = "tickBox",
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerDown = function()
                    settings.autoOpenCodeEditor = not settings.autoOpenCodeEditor
                    tickBox.frame = settings.autoOpenCodeEditor and 2 or 1
                    self.world.func:saveSettings()
                end
            }
        })

        local autoOpenTitle = self.props.content:createChild("Text", {
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
                    settings.autoOpenCodeEditor = not settings.autoOpenCodeEditor
                    tickBox.frame = settings.autoOpenCodeEditor and 2 or 1
                    self.world.func:saveSettings()
                end
            }
        })

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 1,
            onPress = function()
                self.world.props.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                self.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "minimizeButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 4,
            onPress = function(self)
                self.world:minimizeWindow()
            end
        })

        buttonPos = buttonPos - buttonSpacing
        local examplesButton = self.props.content:createChild("UIButton", {
            id = "openExamplesButton",
            toolTip = "toolTip_openExamples",
            x = buttonPos,
            y = 4,
            icon = 20,
            onPress = function(button)
                if button.props.jumpTween then
                    button.props.jumpTween:finishTween()
                    button.props.jumpTween = nil
                end
                if button.props.jumpAction then
                    button.props.jumpAction:destroy()
                    button.props.jumpAction = nil
                end
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openExistingProject",
            x = buttonPos,
            y = 4,
            icon = 6,
            onPress = function(button)
                self.props.enabled = false
                self.world:hideWindow()

                self:wait(0.2):next(function()
                    local path = System:browseDirectory()

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
                    newWindow.func:openWindow()
                    newWindow.func:openDefault()
                    
                    self:destroy()
                end)
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_newProject",
            x = buttonPos,
            y = 4,
            icon = 2,
            onPress = function()
                self.func:closeWindow(function()
                    self.props.enabled = false

                    local newWindow = self.parent:createChild("NewProjectWindow", {
                        x = self.x, y = self.y
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
            end
        })

        if not settings.projects or failedProjects >= #settings.projects then
            self.props.content:createChild("Text", {
                text = Localize:get("label_noProjectsFound"),
                font = "defaultFont",
                color = "#a8bee0",
                origin = 0,
                x = backer.x + 6 + 4,
                y = backer.y + 6
            })

            examplesButton.props.jumpAction = examplesButton:createChild("PeriodicAction", {
                id = "jumping",
                period = 0.7,
                startWithAct = false,
                onAct = function()
                    examplesButton.props.jumpTween = examplesButton.tween:to({
                        y = examplesButton.y - 2,
                        duration = 0.2,
                        ease = Ease.SineOut,
                        yoyo = true,
                        onComplete = function()
                            examplesButton.props.jumpTween = nil
                        end
                    })
                end
            })
        end
    end,

    loadCodeEditors = function(self)
        local settings = self.world.func:getSettings()
        local editors = self.world.func:getCodeEditors()

        self.props.editorMenu.func:createOptions(editors)

        if settings.codeEditor then
            self.props.editorMenu.func:select(settings.codeEditor)
        end
    end
})