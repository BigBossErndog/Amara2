Nodes:define("TextField", "FillRect", {
    width = 256,
    height = 18,
    origin = 0,
    color = "#111d27",
    input = true,

    props = {
        defaultText = Localize:get("label_enterTextHere"),
        inputEnabled = true,
    },

    onConfigure = function(self, config)
        if config.defaultText then
            self.get.defaultText = config.defaultText
        end
        if config.inputEnabled ~= nil then
            self.get.inputEnabled = config.inputEnabled
        end
    end,

    onCreate = function(self)
        self.get.selected = false
        self.get.finalText = ""

        if not self.get.maxTextWidth then
            self.get.maxTextWidth = self.width - 16
        end
        
        self.get.txt = self:createChild("Text",{
            x = 8, y = 2,
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
        end
    end,

    setText = function(self, txt)
        self.get.finalText = txt

        if string.len(txt) <= 0 then
            if self.get.selected then
                self.get.txt.text = txt
                self.get.txt.color = Colors.White
            else
                self.get.txt.text = self.get.defaultText
                self.get.txt.color = "#515f73"
            end
        else
            self.get.txt.text = txt
            self.get.txt.color = Colors.White

            self.get.textInput.text = txt
            while self.get.txt.width > self.get.maxTextWidth do
                self.get.textInput:backspace()
                self.get.txt.text = self.get.textInput.text
                self.get.finalText = self.get.textInput.text
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

    onUpdate = function(self)
        if self.get.selected then
            if self.func:deselect() then
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
    end
})