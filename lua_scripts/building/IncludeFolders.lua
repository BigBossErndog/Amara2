Nodes:define("IncludeFolders", "FillRect", {
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

        self.get.tempData = {}
        local init = false
        if self.get.projectData["build-directories"] then
            for i, v in ipairs(self.get.projectData["build-directories"]) do
                if System:exists(System:join(self.get.projectPath, v)) then
                    self.get.tempData[v] = true
                end
            end
        else
            init = true
        end

        local invalidFolders = {
            build = {},
            lua_scripts = {},
            assets = {}
        }

        local found = System:getSubDirectories(self.get.projectPath)
        self.get.directories = {}
        for i, v in ipairs(found) do
            local dirName = System:getDirectoryName(v)
            if System:isDirectory(v) and (not string.starts_with(dirName, ".")) and (not invalidFolders[dirName]) then
                table.insert(self.get.directories, dirName)
                if init then
                    self.get.tempData[dirName] = true
                end
            end
        end

        for i, v in ipairs(self.get.directories) do
            local opt = self.get.root:createChild("FillRect", {
                x = 0, y = (i - 1)*16,
                width = self.width,
                height = 16,
                color = "#111d27",
                origin = 0,
                props = {
                    dirName = v
                },
                onCreate = function(backer)
                    backer.get.ticker = backer:createChild("Sprite", {
                        origin = 0,
                        x = 8, y = 4,
                        texture = "tickBox",
                        frame = self.get.tempData[backer.get.dirName] and 2 or 1
                    })
                    backer.get.txt = backer:createChild("Text", {
                        x = backer.get.ticker.x + backer.get.ticker.width + 4, y = 2, origin = 0,
                        font = "defaultFont", color = Colors.White,
                        text = v,
                        alpha = self.get.tempData[backer.get.dirName] and 1 or 0.5
                    })
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

        self.get.scrollBar = self.get.content:createChild("FillRect", {
            color = { 80, 80, 80 },
            width = 2,
            origin = 0,
            visible = false,
            
            onCreate = function(scrollBar)
                scrollBar.get.pos = scrollBar:createChild("FillRect", {
                    color = { 200, 200, 200 },
                    width = 2,
                    height = 1,
                    origin = 0
                })

            end,
            manageScrollPosition = function(scrollBar)
                scrollBar.visible = true

                scrollBar.x = self.get.content.x + self.get.content.width + scrollBar.width - 1
                scrollBar.y = self.get.content.y + 2
                
                scrollBar.height = self.get.content.height - 4

                local pos = scrollBar.get.pos

                pos.height = scrollBar.height * ((self.get.content.height - self.get.margin.bottom - self.get.margin.top) / (self.get.wallHeight + self.get.margin.bottom + self.get.margin.top))
                pos.y = -(scrollBar.height - pos.height) * ((-self.get.root.y + self.get.content.top + self.get.margin.top) / ((self.get.content.bottom - self.get.margin.bottom - self.get.wallHeight) - (self.get.content.top + self.get.margin.top)))
            end
        })
    end,

    onUpdate = function(self, deltaTime)
        if self.get.wallHeight > (self.get.content.height - self.get.margin.bottom - self.get.margin.top) then
            self.get.scrollBar.func:manageScrollPosition()
        else
            self.get.scrollBar.visible = false
        end
    end,

    confirmOptions = function(self)
        self.get.projectData["build-directories"] = {}
        for k, v in pairs(self.get.tempData) do
            if v then
                table.insert(self.get.projectData["build-directories"], k)
            end
        end
    end
})