local e = NodeFactory:create("Node")

e.onCreate = function(self)
    print(self, "was created!")
end
e.props.hello = function(self, msg)
    print("Hello from", self, "!")
end

return e -- Make sure to return the entity back to the game