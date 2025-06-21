Project = {
    set = function(self)
        if not self.data then
            self.data = {}
        end
    end,
    is = function(self)
        if not self.data then
            return false
        end
        if self.data == true then
            return true
        end
        return false
    end
}