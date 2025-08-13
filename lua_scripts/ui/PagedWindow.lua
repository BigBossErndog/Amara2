Nodes:define("PagedWindow", "UIWindow", {
    props = {
        pageIndex = 0,
        pageCount = 1
    },

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.props.leftButton = self.props.content:createChild("UIButton", {
            x = 8, y = self.props.targetHeight - 16 - 8,
            icon = 21,
            onPress = function()
                self.func:changePage(-1)
            end
        })

        self.props.rightButton = self.props.content:createChild("UIButton", {
            x = self.props.targetWidth - 16 - 8, y = self.props.targetHeight - 16 - 8,
            icon = 22,
            onPress = function()
                self.func:changePage(1)
            end
        })

        self.props.pageText = self.props.content:createChild("Text", {
            x = self.props.targetWidth / 2.0, y = self.props.targetHeight - 16,
            font = "defaultFont",
            text = "Page " .. self.props.pageIndex .. " / " .. self.props.pageCount,
            color = Colors.White,
            origin = 0.5
        })

        self.func:setPage(1)
    end,

    changePage = function(self, dir)
        local nextIndex = self.props.pageIndex + dir
        if nextIndex < 1 then
            nextIndex = 1
            return
        elseif nextIndex > self.props.pageCount then
            nextIndex = self.props.pageCount
            return
        end
        
        self.func:setPage(nextIndex)
    end,

    setPage = function(self, pageIndex)
        if pageIndex < 1 or pageIndex > self.props.pageCount then
            return false
        end
        self.props.pageIndex = pageIndex

        self.props.leftButton.visible = pageIndex > 1
        self.props.rightButton.visible = pageIndex < self.props.pageCount

        if self.props.pageContent then
            self.props.pageContent:destroy()
        end
        self.props.pageContent = self.props.content:createChild("Group", {
            x = 0, y = 0
        })
        
        if self.func.onCreatePage then
            self.func:onCreatePage(self.props.pageIndex)
        end

        self.props.pageText.text = "Page " .. self.props.pageIndex .. " / " .. self.props.pageCount
    
        return true
    end,
})