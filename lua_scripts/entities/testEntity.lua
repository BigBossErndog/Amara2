local e = factory:create("Entity")

e.props.hello = function(self, msg)
    log("Hello from ", self.id, "!")
end

return e