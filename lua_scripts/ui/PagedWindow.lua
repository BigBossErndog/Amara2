Nodes:define("PagedWindow", "UIWindow", {
    props = {
        pageIndex = 0,
        pageCount = 1
    },

    onCreate = function(self)
        self.classes.UIWindow.func:onCreate()

        self.get.leftButton = self.get.content:createChild("UIButton", {
            x = 8, y = self.get.targetHeight - 16 - 8,
            icon = 21,
            onPress = function()
                self.func:changePage(-1)
            end
        })

        self.get.rightButton = self.get.content:createChild("UIButton", {
            x = self.get.targetWidth - 16 - 8, y = self.get.targetHeight - 16 - 8,
            icon = 22,
            onPress = function()
                self.func:changePage(1)
            end
        })

        self.get.pageText = self.get.content:createChild("Text", {
            x = self.get.targetWidth / 2.0, y = self.get.targetHeight - 16,
            font = "defaultFont",
            text = "Page " .. self.get.pageIndex .. " / " .. self.get.pageCount,
            color = Colors.White,
            origin = 0.5
        })

        self.func:setPage(1)
    end,

    changePage = function(self, dir)
        local nextIndex = self.get.pageIndex + dir
        if nextIndex < 1 then
            nextIndex = 1
            return
        elseif nextIndex > self.get.pageCount then
            nextIndex = self.get.pageCount
            return
        end
        
        self.func:setPage(nextIndex)
    end,

    setPage = function(self, pageIndex)
        if pageIndex < 1 or pageIndex > self.get.pageCount then
            return false
        end
        self.get.pageIndex = pageIndex

        self.get.leftButton.visible = pageIndex > 1
        self.get.rightButton.visible = pageIndex < self.get.pageCount

        if self.get.pageContent then
            self.get.pageContent:destroy()
        end
        self.get.pageContent = self.get.content:createChild("Group", {
            x = 0, y = 0
        })
        
        if self.func.onCreatePage then
            self.func:onCreatePage(self.get.pageIndex)
        end

        self.get.pageText.text = "Page " .. self.get.pageIndex .. " / " .. self.get.pageCount
    
        return true
    end,
})