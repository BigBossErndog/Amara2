local e = factory:create("Entity")
print(e)
e.props.hello = function(self, msg)
    log("Hello from ", self.id, "!")
end

return e