local e = Factory:create("Entity")

e.onCreate = function(self)
    print(self, "was created!")
end
e.props.hello = function(self, msg)
    print("Hello from", self, "!")
end

return e