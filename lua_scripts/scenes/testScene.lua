return NodeFactory:create("Scene"):configure({
    onPreload = function(self) 
        self.load:image("goldenFlower", "goldenFlower.png")
        self.load:spritesheet("freaker", "freaker.png", 32, 64)
        self.load:font("font", "KLEINTEN.ttf", 10)
        
        Animations:add({
            key = "runningDown",
            texture = "freaker",
            startFrame = 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        Animations:add({
            key = "runningUp",
            texture = "freaker",
            startFrame = 10 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        Animations:add({
            key = "runningLeft",
            texture = "freaker",
            startFrame = 20 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        Animations:add({
            key = "runningRight",
            texture = "freaker",
            startFrame = 30 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
    end,

    onCreate = function(self)
        local a_rate = 2 * math.pi * 0.01
        local d_rate = 1
        for i = 1, (64*64) do
            self:createChild("Sprite"):configure({
                texture = "goldenFlower",
                x = math.sin(a_rate * i) * d_rate * i,
                y = math.cos(a_rate * i) * d_rate * i
            })
        end

        local f = self:createChild("Sprite"):configure({
            texture = "freaker",
            animation = "runningDown"
        })
        
        self.props.f = f;
        f.frame = 3

        local txt = self:createChild("Text"):configure({
            text = "abcdefghijklmnopqrstuvwxyz",
            font = "font",
            alignment = Align.Center,
            wordWrap = 40
        })

        local controlRight = Controls:scheme("right");
        controlRight:setKeys({ Key.Right, Key.D });

        local controlLeft = Controls:scheme("left");
        controlLeft:setKeys({ Key.Left, Key.A });

        local controlUp = Controls:scheme("up");
        controlUp:setKeys({ Key.Up, Key.W });

        local controlDown = Controls:scheme("down");
        controlDown:setKeys({ Key.Down, Key.S });

        self.camera:startFollow(f, 10)
    end,

    onUpdate = function(self, deltaTime)
        if Keyboard:isDown(Key.X) then
            self.camera:rotate(2 * math.pi * deltaTime * 0.25)
        end
        if Keyboard:isDown(Key.Z) then
            self.camera:rotate(-2 * math.pi * deltaTime * 0.25)
        end

        if Keyboard:isDown(Key.P) then
            self.camera:changeZoom(1.1 * deltaTime)
        end
        if Keyboard:isDown(Key.O) then
            self.camera:changeZoom(-1.1 * deltaTime)
        end

        self.props.f.rotation = -self.camera.rotation
        
        local speed = 200;

        if Controls:isDown("right") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(speed*deltaTime, 0), -self.camera.rotation))
            self.props.f:animate("runningRight")
        elseif Controls:isDown("left") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(-speed*deltaTime, 0), -self.camera.rotation))
            self.props.f:animate("runningLeft")
        end
        if Controls:isDown("up") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(0, -speed*deltaTime), -self.camera.rotation))
            self.props.f:animate("runningUp")
        elseif Controls:isDown("down") then
            self.props.f:animate("runningDown")
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(0, speed*deltaTime), -self.camera.rotation))
        end

        if Keyboard:justPressed(Key.F5) then
            if self.world.screenMode == ScreenMode.Windowed or self.world.screenMode == ScreenMode.BorderlessWindowed then
                self.world:setScreenMode(ScreenMode.Fullscreen)
            else 
                self.world:setScreenMode(ScreenMode.Windowed)
            end
        end
        if Keyboard:justPressed(Key.F11) then
            if self.world.screenMode == ScreenMode.Windowed or self.world.screenMode == ScreenMode.BorderlessWindowed then
                self.world:setScreenMode(ScreenMode.BorderlessFullscreen)
            else 
                self.world:setScreenMode(ScreenMode.BorderlessWindowed)
            end
        end
    end
});