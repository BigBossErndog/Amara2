Nodes:define("ExampleModule", "Group", {
    onConfigure = function(self, config)
        if config.type then
            self.get.type = config.type
        end
        if config.name then
            self.get.name = config.name
        end
        if config.path then
            self.get.path = config.path
        end
        if config.openProject then
            self.get.openProject = config.openProject
        end
    end,

    onCreate = function(self)
        self.get.rect = self:createChild("FillRect", {
            color = Colors.White,
            x = 8, y = 8,
            width = 56,
            height = 56,
            origin = 0,
            input = {
                active = true,
                cursor = Cursor.Pointer,
                onPointerHover = function()
                    self.get.selected = true
                end,
                onPointerExit = function()
                    self.get.selected = false
                end,
                onPointerUp = function()
                    if self.get.type == "example-project" and self.get.openProject then
                        self.get.openProject(self.get.path)
                    end
                end
            },
            onUpdate = function()
                if self.get.selected then
                    self.get.rect.color = "#333e4d"
                    -- self.world.get.toolTips.func:showToolTip(self.get.projectName, Game.deltaTime, true)
                else
                    self.get.rect.color = "#111d27"
                end
            end
        })
        self.get.cont = self:createChild("Container", {
            width = 56,
            height = 56,
            origin = 0,
            x = 8, y = 8
        })

        self.get.icon = self.get.cont:createChild("Sprite")
        if self.get.type == "example-project" then
            self.get.icon.texture = "largeIcons"
        else
            self.get.icon.texture = "largeIcons"
        end
        self.get.icon:fitWithin({
            x = self.get.cont.left,
            y = self.get.cont.top,
            width = 56,
            height = 56
        })

        self.get.txt = self.get.cont:createChild("Text", {
            font = "defaultFont",
            text = self.get.name,
            origin = { 0.5, 1 },
            y = self.get.cont.bottom - 2
        })
        if self.get.txt.width > self.get.cont.width - 4 then
            self.get.txt.tween:to({
                origin = { 0, 1 },
                x = self.get.cont.left + 2
            })
            self.get.txt.tween:to({
                x = self.get.cont.right - self.get.txt.width - 2,
                duration = 3,
                ease = Ease.SineInOut,
                yoyo = true,
                repeats = -1,
                delay = 1,
                interim = 1
            })
        end
    end
})

Nodes:define("ExamplesWindow", "PagedWindow", {
    width = 64*4 + 8,
    height = 200,

    onCreate = function(self)
        self.get.currentPath = "files/examples"

        self.get.examples = System:getDirectoryContents(self.get.currentPath)
        self.get.pageCount = math.ceil(#self.get.examples / 6)

        self.classes.PagedWindow.func:onCreate()

        local title = self.get.content:createChild("Text", {
            x = 10, y = 8,
            font = "defaultFont",
            text = Localize:get("title_exampleProjects"),
            color = "#f0f6ff",
            input = true,
            origin = 0
        })

        local buttonPos = self.get.targetWidth - 22
        local buttonSpacing = 20

        -- buttonPos = buttonPos - buttonSpacing
        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Backspace },
                { Key.RightAlt, Key.RightShift, Key.Backspace }
            },
            onPress = function()
                self.func:closeWindow(function()
                    self.get.enabled = false
                    
                    local newWindow = self.parent:createChild("MainWindow", {
                        x = self.x, y = self.y
                    })
                    newWindow.func:openWindow()
                    
                    self:destroy()
                end)
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
                self.get.enabled = false
            end,
            hotkey = {
                { Key.LeftAlt, Key.LeftShift, Key.Minus },
                { Key.RightAlt, Key.RightShift, Key.Minus }
            }
        })
    end,

    onCreatePage = function(self, pageIndex)
        if #self.get.examples == 0 then
            return
        end

        local openProject = function(path)
            self.func:closeWindow(function()
                local sandboxPath = "files/sandbox"
                if System:exists(sandboxPath) then
                    System:removeDirectoryContents(sandboxPath)
                end
                System:copy(path, sandboxPath)

                local projectData = System:readJSON(System:join(sandboxPath, "project.json"))
                projectData.uninitiated = true
                projectData.exampleProject = true
                System:writeFile(System:join(sandboxPath, "project.json"), projectData)

                local newWindow = self.parent:createChild("ProjectWindow", {
                    projectPath = sandboxPath
                })
                newWindow.func:openDefault()
                newWindow.func:openWindow()
                
                self:destroy()
            end)
        end

        local startIndex = math.floor((pageIndex - 1) / 6) * 6 + 1
        local endIndex = math.min(startIndex + 5, #self.get.examples)

        for i = startIndex, endIndex do
            local v = self.get.examples[i]

            local type
            local name

            if System:isDirectory(v) then
                if System:exists(System:join(v, "project.json")) then
                    type = "example-project"

                    local projectData = System:readJSON(System:join(v, "project.json"))

                    name = projectData["project-name"]
                else
                    type = "directory"
                    name = System:getFileName(v)
                end

                self.get.pageContent:createChild("ExampleModule", {
                    type = type,
                    name = name,
                    path = v,
                    openProject = openProject,
                    x = ((i - 1) % 3) * 64 + 4,
                    y = math.floor((i - 1) / 3) * 64 + 24
                })
            end
        end
    end
})