local e = factory:create("Entity")

e.onCreate = function(self)
    log(self.entityID, " was created!")
end
e.hello = function(self, msg)
    log("Hello from ", self.id, "!")
end

return e