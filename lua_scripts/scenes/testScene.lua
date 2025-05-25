local freaker
local fpsTxt

return NodeFactory:create("Scene"):configure({
    onPreload = function(self)
        self.load:tilemap("testTilemap", "testTilemap.tmx")
        self.load:image("tiles", "tiles.png")

        self.load:image("goldenFlower", "goldenFlower.png")

        self.load:spritesheet("freaker", "freaker.png", 32, 64)
        self.load:font("font", "KLEINTEN.ttf", 10)

        self.load:audio("music", "battleTheme2.ogg");

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
        -- self:destroy()
    end,

    onCreate = function(self)
        self:createChild("Hotkey", {
            keys = { Key.A, Key.LeftCtrl, Key.LeftAlt },
            onPress = function(self)
                self.world:destroy()
            end
        })
        print(Key.LeftCtrl)

        local sound = self.audio:createChild("Audio", {
            audio = "music",
            -- loop = true,
            playing = true,
            -- volume = 0.5,
        });
        
        sound:wait(3).tween:to({
            volume = 0,
            duration = 2
        })

        local tilemap = self:createChild("Tilemap", {
            texture = "tiles",
            tilemap = "testTilemap"
        })
        self.world.backgroundColor = "black"

        local textCont = self:createChild("TextureContainer", {
            x = tilemap.center.x,
            y = tilemap.center.y,
            width = 256,
            height = 256,
            -- tint = "red",
            -- alpha = 0.5,
            -- visible = false,
            -- paused = true,
            -- canvasLocked = true,
            -- fixedToCamera = true,
            -- origin = Position.Top,
            -- tint = Colors.Red
        })
        print("TextCont", textCont.pos, tilemap.center)
        self.props.textCont = textCont
        textCont:wait(2).tween:to({
            rotation = 2*math.pi,
            duration = 10,
            ease = Ease.QuintInOut,
            repeats = -1,
            yoyo = true
        })
        -- textCont.rect = map.rect
        -- copy.target = textCont

        local a_rate = 2 * math.pi * 0.01
        local d_rate = 1
        for i = 1, (128*128) do
            textCont:createChild("Sprite", {
                texture = "goldenFlower",
                x = math.sin(a_rate * i) * d_rate * i,
                y = math.cos(a_rate * i) * d_rate * i
            })
        end

        -- self.props.txt = textCont:createChild("Text", {
        --     -- text = "a b c d e f g h i j k l m n o p q r s t u v w x y z 0 1 2 3 4 5 6 7 8 9",
        --     text = 123,
        --     font = "font",
        --     wrapMode = WrapMode.ByWord,
        --     wrapWidth = 128,
        --     lineSpacing = 10,
        --     -- fixedToCamera = true,
        --     color = Colors.Red,
        --     x = textCont.left,
        --     y = textCont.top,
        --     -- origin = 1,
        --     alignment = Align.Left,
        -- })

        -- local f = textCont:createChild("Sprite", {
        --     texture = "freaker",
        --     animation = "runningDown",
        -- })
        freaker = self:createChild("Sprite", {
            texture = "freaker",
            animation = "runningDown",
            tint = Colors.Green
            -- origin = 0
        })

        -- f.pos = textCont.center
        -- f:goTo(textCont.center)
        
        freaker.frame = 3

        local controlRight = Controls:scheme("right")
        controlRight:setKeys({ Key.Right, Key.D })

        local controlLeft = Controls:scheme("left")
        controlLeft:setKeys({ Key.Left, Key.A })

        local controlUp = Controls:scheme("up")
        controlUp:setKeys({ Key.Up, Key.W })

        local controlDown = Controls:scheme("down")
        controlDown:setKeys({ Key.Down, Key.S })

        fpsTxt = self:createChild("Text", {
            text = "FPS",
            font = "font",
            origin = 0,
            fixedToCamera = true,
            x = self.camera.left + 10,
            y = self.camera.top + 10,
            color = "white"
        });

        local root = self:createChild("TextureContainer", {
            width = 1280,
            height = 1280,
            fixedToCamera = true
        })
        root:setShaderProgram("outlineShader")

        self.props.prog = root:createChild("Text", {
            text = "Hello ${green_shake}world${end}, I am ${yellow_wave}Amara${end}!",
            font = "font",
            origin = 0,
            fixedToCamera = true,
            x = self.camera.center.x,
            y = self.camera.center.y - 48
        });
        self.props.prog.x = -math.round(self.props.prog.width/2)
        self.props.prog.progress = 0;
        self.props.prog:wait(2):whenDone(function(self)
            self:autoProgress({
                speed = 12, -- How many characters to be shown a second
                onAct = function(self)
                    if Keyboard:justPressed(Key.Space) then
                        self:skipProgress()
                    end
                end
            }):whenDone(function(self)
                print("finished progressing text")
            end)
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

        self.camera:startFollow(freaker, 10)
        
        self.props.checker = self:createChild("Node");
        self.props.checker:createChild("Node", { id = "child1" }):createChild("Node", { id = "child2" }):createChild("Node", { id = "3" })
        print(self.props.checker:get("child1/child2"))
        
        self.camera:setBounds(tilemap.rect)
        -- self.camera.zoom = 5
        -- freaker.pos = tilemap.center
        freaker.blendMode = BlendMode.Mask
        -- textCont.blendMode = BlendMode.Mask
        
        self.props.checked = false
    end,

    onUpdate = function(self, deltaTime)
        fpsTxt:setText(math.floor(Game.fps), " FPS")
        -- fpsTxt:bringToFront()
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

        if Keyboard:justPressed(Key.Space) then
            self.audio:play("music")
        end

        freaker.rotation = -self.camera.rotation
        
        local speed = 200
        local anyPressed = false

        if Controls:isDown("right") then
            freaker:move(math.rotateAroundAnchor(Vector2.new(speed*deltaTime, 0), -self.camera.rotation))
            freaker:animate("runningRight")
            anyPressed = true
        elseif Controls:isDown("left") then
            freaker:move(math.rotateAroundAnchor(Vector2.new(-speed*deltaTime, 0), -self.camera.rotation))
            freaker:animate("runningLeft")
            anyPressed = true
        end
        if Controls:isDown("up") then
            freaker:move(math.rotateAroundAnchor(Vector2.new(0, -speed*deltaTime), -self.camera.rotation))
            freaker:animate("runningUp")
            anyPressed = true
        elseif Controls:isDown("down") then
            freaker:animate("runningDown")
            freaker:move(math.rotateAroundAnchor(Vector2.new(0, speed*deltaTime), -self.camera.rotation))
            anyPressed = true
        end
        -- if not anyPressed then freaker:stopAnimating() end

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