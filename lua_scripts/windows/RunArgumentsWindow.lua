Nodes:define("RunArgumentsWindow", "UIWindow", {
    width = 256,
    height = 150,
    
    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.exampleProject then
            self.get.exampleProject = config.exampleProject
        end
    end,
    
    onCreate = function(self)
        self.get.projectData = System:readJSON(System:join(self.get.projectPath, "project.json"))

        self.super.UIWindow.func:onCreate()

        self.get.wallHeight = 0
        
        self.get.title = self.get.content:createChild("Text", {
            x = 10, y = 6,
            font = "defaultFont",
            text = Localize:get("title_testArguments"),
            color = Colors.White,
            origin = 0
        })
        
        local buttonPos = self.get.targetWidth - 22
        
        self.get.backButton = self.get.content:createChild("UIButton", {
            id = "backButton",
            toolTip = "toolTip_back",
            x = buttonPos,
            y = 4,
            icon = 5,
            onPress = function(button)
                button.get.enabled = false
                self.func:saveArguments()
                self.func:closeWindow(function()
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath,
                        exampleProject = self.get.exampleProject
                    })
                    
                    self:destroy()
                end)
            end
        })
        self:createChild("Hotkey", {
            config = {
                { Key.LeftAlt, Key.LeftShift, Key.Backspace },
                { Key.RightAlt, Key.RightShift, Key.Backspace }
            },
            onPress = function()
                self.get.backButton.func:forcePress()
            end
        })
        
        local backer = self.get.content:createChild("FillRect", {
             x = 6, y = self.get.title.y + 20,
             width = self.get.targetWidth - 12,
             height = 96,
             color = "#111d27",
             origin = 0
        })
        
        self.get.container = self.get.content:createChild("Container", {
            x = backer.x + 2,
            y = backer.y + 2,
            width = backer.width - 4,
            height = backer.height - 4,
            origin = 0
        })

        self.get.playButton = self.get.content:createChild("UIButton", {
            id = "playButton",
            icon = 3,
            onPress = function()
                self.func:saveArguments()
                self.func:closeWindow(function()
                    local newWindow = self.parent:createChild("ProjectWindow", {
                        projectPath = self.get.projectPath,
                        exampleProject = self.get.exampleProject,
                        playOnStart = true
                    })
                    
                    self:destroy()
                end)
            end
        })
        self.get.playButton.pos = {
            x = self.get.targetWidth - self.get.playButton.width - 8,
            y = self.get.targetHeight - self.get.playButton.height - 6
        }
        
        self.get.addArgBtn = self.get.content:createChild("UIButton", {
            text = "label_addArgument",
            onPress = function()
                self.func:addField()
                self.func:updateFields()
                if self.get.wallHeight > self.get.container.height then
                    self.get.root.y = -self.get.wallHeight
                end
            end
        })
        self.get.addArgBtn.pos = {
            x = self.get.playButton.x - self.get.addArgBtn.width - 4,
            y = self.get.playButton.y
        }

        self.get.clearArgsBtn = self.get.content:createChild("UIButton", {
            text = "label_clearArguments",
            onPress = function()
                self.func:clearFields()
                if self.get.wallHeight > self.get.container.height then
                    self.get.root.y = -self.get.wallHeight
                end
            end
        })
        self.get.clearArgsBtn.pos = {
            x = self.get.addArgBtn.x - self.get.clearArgsBtn.width - 4,
            y = self.get.playButton.y
        }
        
        self.get.fields = {}
        
        self.get.root = self.get.container:createChild("Group", {
            x = self.get.container.left,
            y = self.get.container.top
        })

        if self.get.projectData.test_arguments then
            for label, value in pairs(self.get.projectData.test_arguments) do
                self.func:addField()
                local field = self.get.fields[#self.get.fields]
                field.get.label = label
                field.get.value = value == true and "true" or value
                field.get.labelField.func:setText(label)
                field.get.valueField.func:setText(field.get.value)
            end
            self.func:updateFields()
        end

        self.get.scrollBar = self.get.content:createChild("FillRect", {
            color = { 80, 80, 80 },
            width = 4,
            origin = 0,
            visible = false,
            alpha = 0.5,
            
            onCreate = function(scrollBar)
                scrollBar.get.pos = scrollBar:createChild("FillRect", {
                    color = { 200, 200, 200 },
                    width = scrollBar.width,
                    height = 1,
                    origin = 0
                })
            end,
            
            handleScrolling = function(scrollBar)
                local pos = scrollBar.get.pos
                pos.height = scrollBar.height * (self.get.container.height/self.get.wallHeight)
                
                self.get.root.y = self.get.root.y + self.input.mouse.wheel.y * 5
                
                if self.get.scrollDragged then
                    if self.input.pointer.isDown then
                        self.get.scrollBar.alpha = 1
                    else
                        self.get.scrollDragged = false
                        if not self.get.scrollBar.input.hovered then
                            self.get.scrollBar.alpha = 0.5
                        end
                    end
                    local barHeight = (self.get.scrollBar.height - self.get.scrollBar.get.pos.height)
                    local per = (self.input.pointer.y - self.get.scrollBar.get.pos.height/2 - self.get.scrollBar.worldPos.y) / barHeight
                    self.get.root.y = self.get.container.top - per * (self.get.wallHeight - self.get.container.height)
                end

                if self.get.root.y > self.get.container.top then
                    self.get.root.y = self.get.container.top
                elseif self.get.container.height/2 - self.get.root.y > self.get.wallHeight then
                    self.get.root.y = self.get.container.height/2 - self.get.wallHeight
                end
            end,
            
            manageScrollPosition = function(scrollBar)
                scrollBar.visible = true

                scrollBar.x = self.get.container.x + self.get.container.width - scrollBar.width - 1
                scrollBar.y = self.get.container.y + 2
                
                scrollBar.height = self.get.container.height - 4

                scrollBar.get.pos.y = scrollBar.height * ((self.get.container.top - self.get.root.y)/self.get.wallHeight)
            end,
            
            input = {
                active = true,
                onPointerDown = function()
                    self.get.scrollDragged = true
                end,
                onPointerHover = function()
                    self.get.scrollBar.alpha = 1
                end,
                onPointerExit = function()
                    if not self.get.scrollDragged then
                        self.get.scrollBar.alpha = 0.6
                    end
                end
            }
        })

        self.get.noArgs = self.get.container:createChild("Text", {
            x = self.get.container.left + 8, y = self.get.container.top + 2,
            font = "defaultFont",
            text = Localize:get("label_noArguments"),
            color = Colors.White,
            origin = 0,
            visible = #self.get.fields == 0
        })
    end,
    
    addField = function(self)
        local field = self.get.root:createChild("Group")

        field.get.label = nil
        field.get.value = nil
        
        field.get.labelField = field:createChild("TextField", {
            width = 106,
            infiniteLength = true,
            defaultText = "label_enterArgument",
            onTab = function()
                field.get.valueField.func:focusField()
            end,
            color = "#1a2a37",
            onChange = function(f, txt)
                if txt == "" then
                    field.get.label = nil
                else
                    field.get.label = txt
                end
                self.func:saveArguments()
            end
        })
        
        field.get.valueField = field:createChild("TextField", {
            width = 106,
            x = field.get.labelField.width + 2,
            infiniteLength = true,
            defaultText = "label_true",
            color = field.get.labelField.color,
            onChange = function(f, txt)
                if txt == "" then
                    field.get.value = nil
                else
                    field.get.value = txt
                end
                self.func:saveArguments()
            end
        })

        field.get.removeBtn = field:createChild("UIButton", {
            x = field.get.valueField.x + field.get.valueField.width + 2,
            icon = 24,
            toolTip = "toolTip_remove",
            onPress = function()
                self.func:removeField(field)
                self.func:updateFields()
            end
        })

        table.insert(self.get.fields, field)
    end,
    
    updateFields = function(self)
        for i, field in ipairs(self.get.fields) do
            field.get.index = i
            field.y = (i - 1) * 18
        end
        self.get.wallHeight = #self.get.fields * 18
    end,
    
    removeField = function(self, field)
        for i, f in ipairs(self.get.fields) do
            if f == field then
                table.remove(self.get.fields, i)
                break
            end
        end
        field:deactivate()
        self.func:updateFields()
        self.func:saveArguments()
    end,

    clearFields = function(self)
        for i, field in ipairs(self.get.fields) do
            field:deactivate()
        end
        self.get.fields = {}
        self.func:updateFields()
    end,
    
    saveArguments = function(self)
        self.get.projectData.test_arguments = {}
        for i, field in ipairs(self.get.fields) do
            if field.get.label then
                self.get.projectData.test_arguments[field.get.label] = field.get.value and field.get.value or true
            end
        end
        System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
    end,

    onUpdate = function(self)
        if self.get.wallHeight > self.get.container.height then
            self.get.scrollBar.func:handleScrolling()
            self.get.scrollBar.func:manageScrollPosition()

            if self.get.root.y > self.get.container.top then
                self.get.root.y = self.get.container.top
            elseif self.get.root.y + self.get.wallHeight < self.get.container.bottom then
                self.get.root.y = self.get.container.bottom - self.get.wallHeight
            end
        else
            self.get.root.y = self.get.container.top
            self.get.scrollBar.visible = false
        end

        self.get.noArgs.visible = #self.get.fields == 0
    end
})