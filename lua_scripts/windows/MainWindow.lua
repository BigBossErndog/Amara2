return NodeFactory:create("ui/UIWindow", {
    props = {
        onCreate = function(self)
            local props = self.props;
            print(self)

            props.content = self:createChild("Group")

            props.title = self:createChild("Text", {
                font = "defaultFont",
                text = "This is a window.",
                color = Colors.Black,
                origin = 0.5
            })
        end
    }
})