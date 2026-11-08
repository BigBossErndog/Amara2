Nodes:define("BuildArgsList", "FillRect", {
    color = "#111d27",
    origin = 0,

    props = {
        margin = {
            left = 4,
            right = 4,
            top = 4,
            bottom = 4
        },
        padding = {
            left = 2,
            right = 2,
            top = 2,
            bottom = 2
        }
    },

    onConfigure = function(self, config)
        if config.projectPath then
            self.get.projectPath = config.projectPath
        end
        if config.projectData then
            self.get.projectData = config.projectData
        end
        if config.window then
            self.get.window = config.window
        end
    end,

    onCreate = function(self)
        self.get.wallHeight = 0

        self.get.content = self:createChild("Container", {
            x = self.get.margin.left,
            y = self.get.margin.top,
            origin = 0,
            width = self.width - self.get.margin.left - self.get.margin.right,
            height = self.height - self.get.margin.top - self.get.margin.bottom
        })
        self.get.root = self.get.content:createChild("Group", {
            x = self.get.content.left, y = self.get.content.top
        })

        local buildArgs = self.get.projectData.build_args and table.deep_copy(self.get.projectData.build_args) or {}

        self.get.fields = {}
        for label, value in pairs(buildArgs) do
            self.func:addField()
            local field = self.get.fields[#self.get.fields]
            field.get.label = label
            field.get.value = value == true and "true" or value
            field.get.labelField.func:setText(label)
            field.get.valueField.func:setText(field.get.value)
        end
        self.func:updateFields()

        self.get.scrollBar = self.parent:createChild("FillRect", {
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
                pos.height = scrollBar.height * (self.get.content.height/self.get.wallHeight)
                
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
                    self.get.root.y = self.get.content.top - per * (self.get.wallHeight - self.get.content.height)
                end
                
                if self.get.root.y > self.get.content.top then
                    self.get.root.y = self.get.content.top
                elseif self.get.content.height/2 - self.get.root.y > self.get.wallHeight then
                    self.get.root.y = self.get.content.height/2 - self.get.wallHeight
                end
            end,
            
            manageScrollPosition = function(scrollBar)
                scrollBar.visible = true

                scrollBar.x = self.x + self.get.content.x + self.get.content.width - scrollBar.width - 1
                scrollBar.y = self.y + self.get.content.y + 2
                
                scrollBar.height = self.get.content.height - 4

                scrollBar.get.pos.y = scrollBar.height * ((self.get.content.top - self.get.root.y)/self.get.wallHeight)
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

        self.get.addArgBtn = self.get.window.get.pageContent:createChild("UIButton", {
            text = "label_addArgument",
            onPress = function()
                self.func:addField()
                self.func:updateFields()
                if self.get.wallHeight > self.get.content.height then
                    self.get.root.y = -self.get.wallHeight
                end
            end
        })
        self.get.addArgBtn.pos = {
            x = self.x + self.width - self.get.addArgBtn.width,
            y = self.y + self.height + 3
        }

        self.get.clearArgsBtn = self.get.window.get.pageContent:createChild("UIButton", {
            text = "label_clearArguments",
            onPress = function()
                self.func:clearFields()
                if self.get.wallHeight > self.get.content.height then
                    self.get.root.y = -self.get.wallHeight
                end
            end
        })
        self.get.clearArgsBtn.pos = {
            x = self.get.addArgBtn.x - self.get.clearArgsBtn.width - 4,
            y = self.get.addArgBtn.y
        }

        self.get.noArgs = self.get.content:createChild("Text", {
            x = self.get.content.left + 8, y = self.get.content.top + 2,
            font = "defaultFont",
            text = Localize:get("label_noArguments"),
            color = Colors.White,
            origin = 0,
            visible = #self.get.fields == 0
        })
    end,

    confirmOptions = function(self)
        self.func:saveArguments()
    end,

    addField = function(self)
        local field = self.get.root:createChild("Group")

        field.get.label = nil
        field.get.value = nil
        
        field.get.labelField = field:createChild("TextField", {
            width = 100,
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
            width = 100,
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
        self.get.projectData.build_args = {}
        for i, field in ipairs(self.get.fields) do
            if field.get.label then
                self.get.projectData.build_args[field.get.label] = field.get.value and field.get.value or true
            end
        end
        System:writeFile(System:join(self.get.projectPath, "project.json"), self.get.projectData)
    end,

    onUpdate = function(self)
        if self.get.wallHeight > self.get.content.height then
            self.get.scrollBar.func:handleScrolling()
            self.get.scrollBar.func:manageScrollPosition()

            if self.get.root.y > self.get.content.top then
                self.get.root.y = self.get.content.top
            elseif self.get.root.y + self.get.wallHeight < self.get.content.bottom then
                self.get.root.y = self.get.content.bottom - self.get.wallHeight
            end
        else
            self.get.root.y = self.get.content.top
            self.get.scrollBar.visible = false
        end

        self.get.noArgs.visible = #self.get.fields == 0
    end
})