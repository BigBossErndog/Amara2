return NodeFactory:create("TextField", "FillRect", {
    width = 256,
    height = 18,
    origin = 0,
    color = "#111d27",
    input = true,
    props = {
        defaultText = Localize:get("label_enterTextHere"),
        inputEnabled = true,
    },

    configure = function(self, config)
        self:super_configure(config)

        if config.defaultText then
            self.props.defaultText = config.defaultText
        end
        if config.inputEnabled ~= nil then
            self.props.inputEnabled = config.inputEnabled
        end
    end,

    onCreate = function(self)
        self.props.selected = false
        self.props.finalText = ""

        self.props.txt = self.parent:createChild("Text",{
            font = "defaultFont",
            origin = 0,
            wrapMode = WrapMode.ByWord,
            props = {
                backing = self
            },
            onUpdate = function(self)
                self.x = self.props.backing.x + 8
                self.y = self.props.backing.y + 2
            end
        })

        self.props.cursor = self.props.txt:createChild("FillRect", {
            width = 1, height = 10,
            y = 2,
            color = Colors.White,
            origin = 0,
            onCreate = function(self) 
                self.func:hide()
            end,
            onUpdate = function(self, deltaTime)
                if self.props.showing then
                    self.x  = self.parent.width
                    self.props.counter = self.props.counter + deltaTime
                    if self.props.counter >= 0.5 then
                        self.props.counter = self.props.counter - 0.5
                        self.visible = not self.visible
                    end
                end
            end,
            show = function(self)
                self.visible = true
                self.props.showing = true
                self.props.counter = 0
            end,
            hide = function(self)
                self.visible = false
                self.props.showing = false
                self.props.counter = 0
            end
        })

        self.props.textInput = self.props.txt:createChild("TextInput", {
            onInput = function(textInput, txt)
                self.func:setText(txt)
                self.props.cursor.func:show()

                if self.func.onChange then
                    self.func:onChange(txt)
                end
            end
        })

        self.input:listen("onPointerDown", function(self)
            if self.props.inputEnabled then
                self.props.selected = true
                self.props.cursor.func:show()
                self.func:setText(self.props.finalText)
                self.props.textInput:startInput()
            end
        end)

        self.func:setText("")
    end,

    setText = function(self, txt)
        self.props.finalText = txt

        if string.len(txt) <= 0 then
            if self.props.selected then
                self.props.txt.text = txt
                self.props.txt.color = Colors.White
            else
                self.props.txt.text = self.props.defaultText
                self.props.txt.color = "#515f73"
            end
        else
            self.props.txt.text = txt
            self.props.txt.color = Colors.White

            self.props.textInput.text = txt
            while self.props.txt.width > self.width - 16 do
                self.props.textInput:backspace()
                self.props.txt.text = self.props.textInput.text
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
        if self.props.selected then
            if self.func:deselect() then
                self.props.selected = false
                self.props.cursor.func:hide()
                self.props.textInput:stopInput()
                self.func:setText(self.props.finalText)
            end
        end
    end
})