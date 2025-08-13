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
            self.props.projectPath = config.projectPath
        end
        if config.projectData then
            self.props.projectData = config.projectData
        end
    end,

    onCreate = function(self)
        self.props.content = self:createChild("Container", {
            x = self.props.margin.left,
            y = self.props.margin.top,
            origin = 0,
            width = self.width - self.props.margin.left - self.props.margin.right,
            height = self.height - self.props.margin.top - self.props.margin.bottom
        })
        self.props.root = self.props.content:createChild("Group", {
            x = self.props.content.left, y = self.props.content.top
        })

        self.props.tempData = {}
        local init = false
        if self.props.projectData["build-directories"] then
            for i, v in ipairs(self.props.projectData["build-directories"]) do
                if System:exists(System:join(self.props.projectPath, v)) then
                    self.props.tempData[v] = true
                end
            end
        else
            init = true
        end

        local found = System:getSubDirectories(self.props.projectPath)
        self.props.directories = {}
        for i, v in ipairs(found) do
            local dirName = System:getDirectoryName(v)
            if System:isDirectory(v) and dirName ~= "build" and dirName ~= "lua_scripts" then
                table.insert(self.props.directories, dirName)
                if init then
                    self.props.tempData[dirName] = true
                end
            end
        end

        for i, v in ipairs(self.props.directories) do
            self.props.root:createChild("FillRect", {
                x = 0, y = (i - 1)*16,
                width = self.width,
                height = 16,
                color = "#111d27",
                origin = 0,
                props = {
                    dirName = v
                },
                onCreate = function(backer)
                    backer.props.ticker = backer:createChild("Sprite", {
                        origin = 0,
                        x = 8, y = 4,
                        texture = "tickBox",
                        frame = self.props.tempData[backer.props.dirName] and 2 or 1
                    })
                    backer.props.txt = backer:createChild("Text", {
                        x = backer.props.ticker.x + backer.props.ticker.width + 4, y = 2, origin = 0,
                        font = "defaultFont", color = Colors.White,
                        text = v,
                        alpha = self.props.tempData[backer.props.dirName] and 1 or 0.5
                    })
                end,
                input = {
                    active = true,
                    cursor = Cursor.Pointer,
                    onPointerDown = function(box)
                        self.props.tempData[box.props.dirName] = not self.props.tempData[box.props.dirName]
                        box.props.ticker.frame = self.props.tempData[box.props.dirName] and 2 or 1
                        box.props.txt.alpha = self.props.tempData[box.props.dirName] and 1 or 0.5
                    end
                }
            })
        end
    end,

    confirmOptions = function(self)
        self.props.projectData["build-directories"] = {}
        for k, v in pairs(self.props.tempData) do
            if v then
                table.insert(self.props.projectData["build-directories"], k)
            end
        end
    end
})