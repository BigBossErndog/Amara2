Nodes:define("LoadingBar", "FillRect", {
    color = Colors.White,
    origin = 0,
    height = 2,

    onCreate = function(self)
        self.props.x = self.x
        self.props.y = self.y

        self.props.fullWidth = self.width
        self.props.fullHeight = self.height

        self.width = 0
        self.func:tweenIn()
    end,

    tweenIn = function(self)
        self.x = self.props.x
        self.tween:to({
            width = self.props.fullWidth,
            duration = 0.25,
            ease = Ease.SineInOut,
            onComplete = function(self)
                self:wait(0.25):next(function(self)
                    self.func:tweenOut()
                end)
            end
        })
    end,

    tweenOut = function(self)
        self.tween:to({
            x = self.props.x + self.props.fullWidth,
            width = 0,
            duration = 0.25,
            ease = Ease.SineInOut,
            onComplete = function(self)
                self:wait(0.25):next(function(self)
                    self.func:tweenIn()
                end)
            end
        })
    end
})