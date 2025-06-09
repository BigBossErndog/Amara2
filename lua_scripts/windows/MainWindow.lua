return NodeFactory:create("ui/UIWindow", {
    props = {
        onCreateWindow = function(self)
            local props = self.props;

            props.content = self:createChild("Group", {
                visible = false;
            })

            props.title = self.content:createChild("Text", {
                font = "defaultFont",
                text = "This is a window.",
                color = Colors.Black,
                origin = 0.5
            })
        end
    }
})