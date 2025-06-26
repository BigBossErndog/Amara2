return Nodes:create("MainWindow", "UIWindow", {
    width = 256,
    height = 140,
    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        local title = self.props.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_recentProjects"),
            color = "#f0f6ff",
            origin = 0,
            input = true
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
        if settings.projects then
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
                        input = true,
                        props = {
                            projectPath = projectPath
                        }
                    })
                    optBacker.input:listen("onPointerHover", function(self)
                        self.color = "#333e4d"
                    end)
                    optBacker.input:listen("onPointerExit", function(self)
                        self.color = "#111d27"
                    end)
                    optBacker.input:listen("onPointerUp", function(self)
                        self.input:deactivate()
                        window.func:closeWindow(function()
                            local newWindow = window.parent:createChild("ProjectWindow", {
                                projectPath = self.props.projectPath
                            })
                            newWindow.func:openWindow()
                            newWindow.func:openDefault()
                            
                            window:destroy()
                        end)
                    end)

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
            y = backer.y + backer.height + 4,
            origin = 0,
            font = "defaultFont",
            color = Colors.White,
            text = Localize:get("title_codeEditor")
        })

        local editorMenu = self.props.content:createChild("DropDownMenu", {
            x = backer.x,
            y = editorTitle.y + editorTitle.height + 4,
            width = backer.width,
            defaultText = Localize:get("label_noCodeEditorAvailable")
        })
        self.props.editorMenu = editorMenu

        self.func:loadCodeEditors()

        if settings.codeEditor then
            editorMenu.func:select(settings.codeEditor)
        end

        local buttonPos = self.props.targetWidth - 22
        local buttonSpacing = 20

        self.props.content:createChild("UIButton", {
            id = "exitButton",
            toolTip = "toolTip_exit",
            x = buttonPos,
            y = 4,
            icon = 0,
            onPress = function()
                self.world.props.windows.func:closeAll(function(self)
                    self.world:destroy()
                end)
                self.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "newProjectButton",
            toolTip = "toolTip_minimize",
            x = buttonPos,
            y = 4,
            icon = 3,
            onPress = function(self)
                self.world:minimizeWindow()
                self.props.enabled = false
            end
        })

        buttonPos = buttonPos - buttonSpacing
        self.props.content:createChild("UIButton", {
            id = "openDirectoryButton",
            toolTip = "toolTip_openExistingProject",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                self.props.enabled = false

                self.world.screenMode = ScreenMode.BorderlessWindowed
                self.world:minimizeWindow()

                self:wait(0.2):next(function()
                    local path = System:browseDirectory()

                    self.world:restoreWindow()
                    self.world.screenMode = ScreenMode.BorderlessFullscreen

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
            icon = 1,
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
    end,

    loadCodeEditors = function(self)
        local editors = self.world.props.editors

        if editors then
            self.props.editorMenu.func:createOptions(editors)
        end
    end
})