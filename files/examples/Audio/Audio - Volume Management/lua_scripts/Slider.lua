Nodes:define("Slider", "Group", {
    props = {
        value = 1
    },

    onConfigure = function(self, config)
        if config.onChange then
            self.get.onChange = config.onChange
        end
        if config.text then
            self.get.text = config.text
        end
        if config.value then
            self.get.value = config.value
        end
    end,
    
    onCreate = function(self)
        self.get.backing = self:createChild("FillRect", {
            color = Colors.Gray,
            width = 200,
            height = 6,
            origin =  { 0, 0.5 },
            x = -100,

            input = {
                active = true,
                cursor = Cursor.Pointer,

                onPointerDown = function()
                    self.get.dragging = true
                end
            }
        })

        self.get.front = self:createChild("FillRect", {
            width = self.get.backing.width * self.get.value,
            height = 6,
            origin =  { 0, 0.5 },
            x = self.get.backing.x
        })
        
        if self.get.text then
            self.get.txt = self:createChild("Text", {
                font = "defaultFont",
                text = self.get.text,
                y = -16
            })
        end
    end,

    setValue = function(self, val)
        local oldValue = self.get.value

        self.get.value = math.max(val, 0)
        self.get.value = math.min(self.get.value, 1)

        if self.get.value == oldValue then
            return
        end
        
        self.get.front.width = self.get.value * self.get.backing.width

        if self.get.onChange then
            self.get.onChange(self.get.value)
        end
    end,

    onUpdate = function(self, deltaTime)
        if self.get.dragging then
            if self.input.pointer.isDown then
                self.func:setValue((self.input.pointer.x - self.get.backing.worldPos.x) / self.get.backing.width)
            else
                self.get.dragging = false
            end
        end
    end
})