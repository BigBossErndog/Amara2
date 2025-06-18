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
            origin = 0,
            onUpdate = function(self)
                self:setText(Game.FPS)
            end,
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
                self.call:closeBox(function(self)
                    self.world:destroy()
                end)
            end
        })
    end
})