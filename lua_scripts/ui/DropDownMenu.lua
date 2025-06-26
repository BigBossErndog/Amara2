return Nodes:create("DropDownMenu", "FillRect", {
    width = 256,
    height = 16,
    origin = 0,
    color = "#111d27",
    input = true,
    
    props = {
        defaultText = Localize:get("label_empty"),
        inputEnabled = true
    },

    onConfigure = function(self, config)
        if config.options then
            self.props.options = config.options
        end

        if config.defaultText then
            self.props.defaultText = config.defaultText
        end

        if config.inputEnabled ~= nil then
            self.props.inputEnabled = config.inputEnabled
        end
    end,

    onCreate = function(self)   
        self.props.txt = self:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            x = 10, y = 1,
            color = "#a8bee0",
            text = self.props.defaultText
        })

        if self.props.options then
            self.func:createOptions(self.props.options)
        end

        self.input:listen("onPointerDown", function(self)
            self.func:openMenu()
        end)
    end,

    createOptions = function(self, options)
        if options and #options > 0 then
            self.props.menu = self:createChild("Group", {
                y = self.height + 2,
                visible = false
            })

            self.props.menuOptions = {}

            local optHeight = 16

            for i = 1, #options do
                local str = options[i]
                
                local backer = self.props.menu:createChild("FillRect", {
                    width = self.width,
                    height = optHeight,
                    y = (i - 1) * optHeight,
                    origin = 0,
                    color = self.color,
                    input = true
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
                backer.props.txt = txt

                backer.props.opt = str

                backer.input:listen("onPointerHover", function()
                    backer.color = "#333e4d"
                end)
                backer.input:listen("onPointerExit", function()
                    backer.color = self.color
                end)
                backer.input:listen("onPointerUp", function()
                    self.props.menu.visible = false
                    self.func:select(backer.props.opt)

                    self.props.dropIcon.frame = 8
                end)

                table.insert(self.props.menuOptions, txt)
            end

            if options and #options > 0 then
                local dropIcon = self:createChild("Sprite", {
                    texture = "uiIcons",
                    origin = 0.5,
                    frame = 8
                })
                dropIcon.x = self.width - dropIcon.width/2.0 - 4
                dropIcon.y = self.height/2.0
                
                self.props.dropIcon = dropIcon
            end
        end
    end,

    openMenu = function(self)
        if self.props.menu then
            self.props.menu.visible = not self.props.menu.visible
            if self.props.dropIcon then
                if self.props.menu.visible then
                    self.props.dropIcon.frame = 9
                else
                    self.props.dropIcon.frame = 8
                end
            end
        end
    end,

    select = function(self, str)
        if Localize:has(str) then
            self.props.txt.text = Localize:get(str)
        else
            self.props.txt.text = str
        end

        self.props.selected = str

        if self.func.onSelect then
            self.func:onSelect(str)
        end
    end
})