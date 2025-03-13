entity = Entity.instance()

entity.create = function(self) {
    self.props.child = game.factory.create("")
}

entity.update = function(self)
    self.props.child.x += 1
end