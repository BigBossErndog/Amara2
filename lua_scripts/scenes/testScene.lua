return NodeFactory:create("Scene"):configure({
    onPreload = function(self) 
        self.load:image("goldenFlower", "goldenFlower.png")
        self.load:image("bigThing", "CrossroadCommune-EXT.png")
        self.load:spritesheet("freaker", "freaker.png", 32, 64)
        self.load:font("font", "KLEINTEN.ttf", 10)

        self.animations:add({
            key = "runningDown",
            texture = "freaker",
            startFrame = 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        self.animations:add({
            key = "runningUp",
            texture = "freaker",
            startFrame = 10 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        self.animations:add({
            key = "runningLeft",
            texture = "freaker",
            startFrame = 20 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
        self.animations:add({
            key = "runningRight",
            texture = "freaker",
            startFrame = 30 + 3,
            numFrames = 6,
            frameRate = 12,
            repeats = -1
        })
    end,

    onCreate = function(self)
        local map = self:createChild("Sprite"):configure({
            texture = "bigThing",
            fixedToCamera = true
            -- alpha = 0.5
        })
        map:fitWithin(self.camera.view)
        
        map = self:createChild("Sprite"):configure({
            texture = "bigThing",
            -- alpha = 0.5
        })
        self.camera:setBounds(map.rect)

        local textCont = self:createChild("TextureContainer", {
            width = 256,
            height = 256,
            -- alpha = 0.5,
            -- visible = false,
            -- paused = true,
            -- canvasLocked = true,
            -- fixedToCamera = true,
            -- origin = Position.Top,
            tint = Colors.Red
        })
        self.props.textCont = textCont
        textCont:wait(2).tween:to({
            rotation = 2*math.pi,
            duration = 10,
            ease = Ease.QuintInOut,
            repeats = -1,
            yoyo = true
        })
        textCont:fitWithin(self.camera.view)
        -- textCont.rect = map.rect
        
        local a_rate = 2 * math.pi * 0.01
        local d_rate = 1
        for i = 1, (128*128) do
            textCont:createChild("Sprite", {
                texture = "goldenFlower",
                x = math.sin(a_rate * i) * d_rate * i,
                y = math.cos(a_rate * i) * d_rate * i
            })
        end

        self.props.txt = textCont:createChild("Text", {
            -- text = "a b c d e f g h i j k l m n o p q r s t u v w x y z 0 1 2 3 4 5 6 7 8 9",
            text = 123,
            font = "font",
            wrapMode = WrapMode.ByWord,
            wrapWidth = 128,
            lineSpacing = 10,
            -- fixedToCamera = true,
            color = Colors.Yellow,
            x = textCont.left,
            y = textCont.top,
            -- origin = 1,
            alignment = Align.Left
        })

        local f = textCont:createChild("Sprite", {
            texture = "freaker",
            animation = "runningDown",
        })
        local f = self:createChild("Sprite", {
            texture = "freaker",
            animation = "runningDown",
            tint = Colors.Green
            -- origin = 0
        })

        f.pos = textCont.center
        -- f:goTo(textCont.center)
        
        self.props.f = f;
        f.frame = 3

        local controlRight = Controls:scheme("right")
        controlRight:setKeys({ Key.Right, Key.D })

        local controlLeft = Controls:scheme("left")
        controlLeft:setKeys({ Key.Left, Key.A })

        local controlUp = Controls:scheme("up")
        controlUp:setKeys({ Key.Up, Key.W })

        local controlDown = Controls:scheme("down")
        controlDown:setKeys({ Key.Down, Key.S })

        self.props.fpsTxt = self:createChild("Text", {
            text = "FPS",
            font = "font",
            origin = 0,
            fixedToCamera = true,
            x = self.camera.left + 10,
            y = self.camera.top + 10,
            color = Colors.Red
        });

        self.props.prog = self:createChild("Text", {
            text = "Hello ${green_shake}world${reset}, I am ${yellow_wave}Amara${reset}!",
            font = "font",
            origin = 0.5,
            fixedToCamera = true,
            x = self.camera.center.x,
            y = self.camera.center.y - 48
        });
        self.props.prog.progress = 0;
        self.props.prog:wait(2):whenDone(function(self)
            self:autoProgress({
                speed = 12,
                onAct = function(self)
                    if Keyboard:justPressed(Key.Space) then
                        self:skipProgress()
                    end
                end
            })
        end)

        self.props.prog:setManipulator("yellow_wave", function(index, lifeTime, character)
            return {
                offsetY = math.sin(index + lifeTime*5),
                color = "yellow"
            }
        end)
        self.props.prog:setManipulator("green_shake", function(index, lifeTime, character)
            return {
                offsetX = math.random(),
                offsetY = math.random(),
                color = "green"
            }
        end)

        self.camera:startFollow(f, 10)

        self.props.checker = self:createChild("Node");
        self.props.checker:createChild("Node", { id = "child1" }):createChild("Node", { id = "child2" }):createChild("Node", { id = "3" })
        print(self.props.checker:getChild("child1/child2"))
    end,

    onUpdate = function(self, deltaTime)
        self.props.fpsTxt:setText(math.floor(Game.fps), " FPS")
        -- self.props.fpsTxt:bringToFront()
        -- print(math.floor(Game.fps), " FPS")
        -- self.props.textCont.y = self.camera.topBorder

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
        
        local speed = 200
        local anyPressed = false

        if Controls:isDown("right") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(speed*deltaTime, 0), -self.camera.rotation))
            self.props.f:animate("runningRight")
            anyPressed = true
        elseif Controls:isDown("left") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(-speed*deltaTime, 0), -self.camera.rotation))
            self.props.f:animate("runningLeft")
            anyPressed = true
        end
        if Controls:isDown("up") then
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(0, -speed*deltaTime), -self.camera.rotation))
            self.props.f:animate("runningUp")
            anyPressed = true
        elseif Controls:isDown("down") then
            self.props.f:animate("runningDown")
            self.props.f:move(math.rotateAroundAnchor(Vector2.new(0, speed*deltaTime), -self.camera.rotation))
            anyPressed = true
        end
        -- if not anyPressed then self.props.f:stopAnimating() end

        if Keyboard:justPressed(Key.F5) then
            if self.world.screenMode == ScreenMode.Windowed or self.world.screenMode == ScreenMode.BorderlessWindowed then
                self.world.screenMode = ScreenMode.Fullscreen
            else 
                self.world.screenMode = ScreenMode.Windowed
            end
        end
        if Keyboard:justPressed(Key.F11) then
            if self.world.screenMode == ScreenMode.Windowed or self.world.screenMode == ScreenMode.BorderlessWindowed then
                self.world.screenMode = ScreenMode.BorderlessFullscreen
            else 
                self.world.screenMode = ScreenMode.BorderlessWindowed
            end
        end
    end
});