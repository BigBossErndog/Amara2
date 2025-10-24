-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "StateMachine"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
        self.load:image("stickman", "stickman.png")
    end,

    onCreate = function(self)
        self.get.myText = self:createChild("Text", {
            font = "defaultFont",
            text = "Click to change state.",
            y = 32
        })

        self.get.stateMachine = self:createChild("StateMachine")

        self.get.stickman = self:createChild("Sprite", {
            texture = "stickman"
        })
    end,

    onUpdate = function(self, deltaTime)
        local s = self.get.stateMachine

        if s:state("start") then -- Automatically enters the first state defined.
            
            if s:event() then
                if self.input.pointer.justPressed then
                    s:nextEvent()
                end
            end

            s:switchStateEvent("spinning")
        
        elseif s:state("spinning") then
            if s:event() then
                self.get.stickman:rotate(0.2) -- Rotate while in this event.

                if self.input.pointer.justPressed then
                    s:nextEvent() -- nextEvent before a switchState so that we return to this state at the next event.
                    
                    s:switchState("jumping") -- Switch state inside an event.
                end
            end

            s:switchStateEvent("shaking")

            s:restartStateEvent() --  Restart the state again.

        elseif s:state("jumping") then
            if s:once() then
                self.get.stickman.rotation = 0

                self.get.stickman.tween:to({
                    y = -32,
                    duration = 0.3,
                    ease = Ease.SineOut,
                    yoyo = true,
                    onComplete = function()
                        s:nextEvent()
                    end
                })
            end
            s:event() -- Holds the state until the tween finishes.

            s:returnStateEvent() -- Goes back to the previous state.

        elseif s:state("shaking") then
            self.get.stickman.pos = {
                x = math.random(-5, 5),
                y = math.random(-5, 5)
            }

            s:wait(0.5) -- Shake for 0.5 seconds.

            s:returnStateEvent()
        end
    end
})