local e = Factory:create("Entity")

e.onCreate = function(self)
    log(self, " was created!")
end
e.props.hello = function(self, msg)
    log("Hello from ", self, "!")
end

e.x = 100;
log(e)

return e