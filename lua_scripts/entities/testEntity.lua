local e = world.factory:create("Entity")

e.id = "WE"

e.props.hello = function(self, msg)
    -- log("Hello from ", self.props.name, "!")
    print(msg)
    print(self.id)
end

e.props.wtf = "WTF"

return e