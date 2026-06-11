Nodes:define("CheckingBox", "FillRect", {
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
        },
        record = {}
    },

    onConfigure = function(self, config)
        if config.options then
            self.get.options = config.options
        end
        if config.record then
            self.get.record = config.record
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

        for i, v in ipairs(self.get.options) do
            local opt = self.get.root:createChild("FillRect", {
                x = 0,
                y = (i - 1) * 16,
                width = self.get.content.width - 8,
                height = 16,
                color = "#111d27",
                origin = 0,
                props = {
                    dirName = v
                },
                onCreate = function(backer)
                    backer.get.ticker = backer:createChild("Sprite", {
                        origin = 0,
                        x = 8,
                        y = 4,
                        texture = "tickBox",
                        frame = self.get.tempData[backer.get.dirName] and 2 or 1
                    })
                    backer.get.txt = backer:createChild("Text", {
                        x = backer.get.ticker.x + backer.get.ticker.width + 4,
                        y = 2,
                        origin = 0,
                        font = "defaultFont",
                        color = Colors.White,
                        -- text = v,
                        alpha = self.get.tempData[backer.get.dirName] and 1 or 0.5
                    })
                    backer.get.txt.text = Localize:get(v.text)
                end,
                input = {
                    active = true,
                    cursor = Cursor.Pointer,
                    onPointerDown = function(box)
                        self.get.tempData[box.get.dirName] = not self.get.tempData[box.get.dirName]
                        box.get.ticker.frame = self.get.tempData[box.get.dirName] and 2 or 1
                        box.get.txt.alpha = self.get.tempData[box.get.dirName] and 1 or 0.5
                    end
                }
            })
            if opt.y + opt.height > self.get.wallHeight then
                self.get.wallHeight = opt.y + opt.height
            end
        end

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
    end,

    onUpdate = function(self, deltaTime)
        if self.get.wallHeight > self.get.content.height then
            self.get.scrollBar.func:handleScrolling()
            self.get.scrollBar.func:manageScrollPosition()
        else
            self.get.scrollBar.visible = false
        end
    end,

    confirmOptions = function(self)
        
    end
})