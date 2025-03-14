local entity = factory:create("Entity")

entity.id = "testEntity"
entity.props.what = "WHAT"

entity.props.hello = function(self, msg)
    return self.props.what
end

-- entity.props.wtf("Hello!")

return entity;