Nodes:define("LoadingBar", "FillRect", {
    color = Colors.White,
    origin = 0,
    height = 2,

    onCreate = function(self)
        self.get.x = self.x
        self.get.y = self.y

        self.get.fullWidth = self.width
        self.get.fullHeight = self.height

        self.width = 0
        self.func:tweenIn()
    end,

    tweenIn = function(self)
        self.x = self.get.x
        self.tween:to({
            width = self.get.fullWidth,
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
            x = self.get.x + self.get.fullWidth,
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