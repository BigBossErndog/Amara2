Nodes:define("TextField", "FillRect", {
    width = 256,
    height = 18,
    origin = 0,
    color = "#111d27",
    input = {
        active = true,
        cursor = Cursor.Text
    },

    props = {
        defaultText = "label_enterTextHere",
        inputEnabled = true,
        fixLengthToSize = true,
        skip = false
    },

    onConfigure = function(self, config)
        if config.defaultText then
            self.get.defaultText = config.defaultText
        end
        if config.inputEnabled ~= nil then
            self.get.inputEnabled = config.inputEnabled
        end
        if config.fixLengthToSize ~= nil then
            self.get.fixLengthToSize = config.fixLengthToSize
        end
        if config.maxTextWidth then
            self.get.maxTextWidth = config.maxTextWidth
        end
        if config.infiniteLength then
            self.get.infiniteLength = config.infiniteLength
            self.get.fixLengthToSize = false
        end
        if config.toolTip then
            self.get.toolTip = config.toolTip
        end
    end,

    onCreate = function(self)
        self.get.selected = false
        self.get.finalText = ""
        
        if self.get.fixLengthToSize then
            self.get.maxTextWidth = self.width - 16
        end
        
        self.get.container = self:createChild("TextureContainer", {
            x = 2,
            y = 0,
            width = self.width - 4,
            height = self.height,
            origin = 0
        })
        
        self.get.root = self.get.container:createChild("Group", {
            x = self.get.container.left,
            y = self.get.container.top
        })
        
        self.get.txt = self.get.root:createChild("Text",{
            x = 6, y = 2,
            font = "defaultFont",
            origin = 0,
            wrapMode = WrapMode.ByWord,
            props = {
                backing = self
            }
        })

        self.get.cursor = self.get.txt:createChild("FillRect", {
            width = 1, height = 10,
            y = 2,
            color = Colors.White,
            origin = 0,
            onCreate = function(self)
                self.func:hide()
            end,
            onUpdate = function(self, deltaTime)
                if self.get.showing then
                    self.x  = self.parent.width
                    self.get.counter = self.get.counter + deltaTime
                    if self.get.counter >= 0.5 then
                        self.get.counter = self.get.counter - 0.5
                        self.visible = not self.visible
                    end
                end
            end,
            show = function(self)
                self.visible = true
                self.get.showing = true
                self.get.counter = 0
            end,
            hide = function(self)
                self.visible = false
                self.get.showing = false
                self.get.counter = 0
            end
        })

        self.get.textInput = self.get.txt:createChild("TextInput", {
            onInput = function(textInput, txt)
                self.func:setText(txt)
                self.get.cursor.func:show()

                if self.func.onChange then
                    self.func:onChange(self.get.finalText)
                end
            end
        })

        self.input:listen("onPointerDown", function(self)
            self.func:focusField()
        end)

        self.func:setText("")
    end,
    
    focusField = function(self)
        if self.get.inputEnabled then
            self.get.selected = true
            self.get.cursor.func:show()
            self.func:setText(self.get.finalText)
            self.get.textInput:startInput()

            if self.func.onFocus then
                self.func:onFocus(self.get.finalText)
            end
            self.get.skip = true
        end
    end,

    setText = function(self, txt)
        self.get.finalText = txt

        if string.len(txt) <= 0 then
            if self.get.selected then
                self.get.txt.text = txt
                self.get.txt.color = Colors.White
            else
                self.get.txt.text = Localize:get(self.get.defaultText)
                self.get.txt.color = "#515f73"
            end
        else
            self.get.txt.text = txt
            self.get.txt.color = Colors.White

            self.get.textInput.text = txt
            if self.get.maxTextWidth then
                while self.get.txt.width > self.get.maxTextWidth do
                    self.get.textInput:backspace()
                    self.get.txt.text = self.get.textInput.text
                    self.get.finalText = self.get.textInput.text
                end
            end
        end
    end,

    deselect = function(self)
        if self.input.mouse.left.justPressed and not self.input.hovered then
            return true
        end
        if Keyboard:justPressed(Key.Esc) then
            return true
        end
        if Keyboard:justPressed(Key.Enter) then
            return true
        end
        return false
    end,

    onUpdate = function(self, deltaTime)
        if self.get.skip then
            self.get.skip = false
            return
        end
        self.input.cursor = self.get.inputEnabled and Cursor.Text or Cursor.Default
        if self.get.selected then
            local tabbed = false

            if Keyboard:justPressed(Key.Tab) then
                tabbed = true
                if self.func.onTab then
                    self.func:onTab(self.get.finalText)
                end
            end

            if tabbed or self.func:deselect() then
                self.get.selected = false
                self.get.cursor.func:hide()
                self.get.textInput:stopInput()

                if self.func.onUnfocus then
                    self.func:onUnfocus(self.get.finalText)
                end

                self.func:setText(self.get.finalText)

                if Keyboard:justPressed(Key.Enter) then
                    if self.func.onEnter then
                        self.func:onEnter(self.get.finalText)
                    end
                end
            end
        end

        if self.get.toolTip and self.input.hovered then
            self.world.get.toolTips.func:showToolTip(self.get.toolTip, deltaTime)
        end
    end,
    
    onPreDraw = function(self)
        if self.get.selected then
            if self.get.txt.width > self.get.container.width - 12 then
                self.get.txt.x = 6 - (self.get.txt.width - (self.get.container.width - 12))
            else
                self.get.txt.x = 6
            end
        end
    end
})