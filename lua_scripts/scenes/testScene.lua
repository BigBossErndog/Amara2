scene = Factory:create("Scene")

scene.onPeload = function(self) 
    self.load.image()
end

scene.onCreate = function(self)
end

scene.onUpdate = function(self, deltaTime)
end

return scene;