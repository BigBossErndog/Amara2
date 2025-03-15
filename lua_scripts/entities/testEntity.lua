local entity = game.factory:create("Entity")

entity.id = "testEntity"

entity.props.hello = function(self, msg)
    log(self.id)
end

return entity;