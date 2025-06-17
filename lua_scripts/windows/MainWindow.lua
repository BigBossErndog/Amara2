return NodeFactory:create("UIWindow", {
    onCreate = function(self)
        self.classes.UIWindow.super:onCreate()

        local props = self.props;
        
        props.content = self:createChild("Group", {
            x = self.left,
            y = self.top,
            visible = false;
        })

        props.title = props.content:createChild("Text", {
            x = 4, y = 4,
            font = "defaultFont",
            text = "This is a window.",
            color = Colors.Black,
            origin = 0
        })
    end
})