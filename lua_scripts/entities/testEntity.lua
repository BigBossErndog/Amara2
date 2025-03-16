local e = game.factory:create("Entity")

e.props.hello = function(self) 
    log(self.id)
end

return e