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
            text = Localize:get("amara2_title"),
            color = Colors.Black,
            origin = 0,
            input = true
        })
        props.title.input:listen("onPointerHover", function(self)
            self.color = Colors.Blue
        end)
        props.title.input:listen("onPointerExit", function(self)
            self.color = Colors.Black
        end)

        self:createChild("Hotkey", {
            keys = { Key.LeftCtrl, Key.LeftAlt, Key.A },
            onPress = function(self)
                self.props.content.visible = false
                self.func:closeBox(function(self)
                    self.world:destroy()
                end)
            end
        })
    end
})