local e = NodeFactory:create("Node")

e.onCreate = function(self)
    print(self, "was created!")
end
e.bind.hello = function(self, msg)
    e.call:onCreate()
    print("Hello from", self, "!")
end

return e