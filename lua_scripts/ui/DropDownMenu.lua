Nodes:define("DropDownMenu", "FillRect", {
    width = 256,
    height = 16,
    origin = 0,
    color = "#111d27",
    
    props = {
        defaultText = Localize:get("label_empty"),
        inputEnabled = true
    },

    input = {
        active = true,
        cursor = Cursor.Pointer,
        onPointerDown = function(self, pointer)
            if self.get.inputEnabled then
                self.func:openMenu()
            end
        end
    },

    onConfigure = function(self, config)
        if config.options then
            self.get.options = config.options
        end

        if config.defaultText then
            self.get.defaultText = config.defaultText
        end

        if config.inputEnabled ~= nil then
            self.get.inputEnabled = config.inputEnabled
        end
    end,

    onCreate = function(self)   
        self.get.txt = self:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            x = 10, y = 1,
            color = "#a8bee0",
            text = self.get.defaultText
        })

        if self.get.options then
            self.func:createOptions(self.get.options)
        end
    end,

    createOptions = function(self, options)
        self.get.menuOptions = {}
        if self.get.menu then
            self.get.menu:destroy()
            self.get.menu = nil
        end

        if options and #options > 0 then
            self.get.menu = self:createChild("Group", {
                y = self.height + 2,
                visible = false
            })

            local optHeight = 16

            for i = 1, #options do
                local str = options[i]
                
                local backer = self.get.menu:createChild("FillRect", {
                    width = self.width,
                    height = optHeight,
                    y = (i - 1) * optHeight,
                    origin = 0,
                    color = self.color,
                    input = {
                        active = true,
                        cursor = Cursor.Pointer
                    }
                })

                local txt = backer:createChild("Text", {
                    x = 10, y = 1,
                    font = "defaultFont",
                    origin = 0,
                    color = "#a8bee0",
                })
                if Localize:has(str) then
                    txt.text = Localize:get(str)
                else
                    txt.text = str
                end
                backer.get.txt = txt

                backer.get.opt = str

                backer.input:listen("onPointerHover", function()
                    backer.color = "#333e4d"
                end)
                backer.input:listen("onPointerExit", function()
                    backer.color = self.color
                end)
                backer.input:listen("onPointerUp", function()
                    self.get.menu.visible = false
                    self.func:select(backer.get.opt)

                    self.get.dropIcon.frame = 9
                end)

                table.insert(self.get.menuOptions, txt)
            end

            if options and #options > 0 then
                if not self.get.dropIcon then
                    local dropIcon = self:createChild("Sprite", {
                        texture = "uiIcons",
                        origin = 0.5
                    })
                    dropIcon.x = self.width - dropIcon.width/2.0 - 4
                    dropIcon.y = self.height/2.0
                    
                    self.get.dropIcon = dropIcon
                end
                self.get.dropIcon.frame = 9
            else
                if self.get.dropIcon then
                    self.get.dropIcon.visible = false
                end
            end
        end
    end,

    openMenu = function(self)
        if self.get.menu then
            self.get.menu.visible = not self.get.menu.visible
            if self.get.dropIcon then
                if self.get.menu.visible then
                    self.get.dropIcon.frame = 10
                else
                    self.get.dropIcon.frame = 9
                end
            end
        end
    end,

    select = function(self, str)
        if Localize:has(str) then
            self.get.txt.text = Localize:get(str)
        else
            self.get.txt.text = str
        end

        self.get.selected = str

        if self.func.onSelect then
            self.func:onSelect(str)
        end
    end
})