scene = NodeFactory:create("Scene")

scene.onPreload = function(self) 
    self.load:image("goldenFlower", "goldenFlower.png")
    self.load:spritesheet("freaker", "freaker.png", 32, 64);
    
    Animations:add({
        key = "running",
        texture = "freaker",
        startFrame = 3,
        numFrames = 6,
        frameRate = 12,
        repeats = -1
    })
end

scene.onCreate = function(self)
    Game:setTargetFPS(60)

    local s = self:createChild("Sprite")
    s:setTexture("goldenFlower")
    s.id = "test"

    s.scale.x = 2
    s.scale.y = 2

    -- s.cropLeft = 70

    local t = s:createChild("Sprite")
    t:setTexture("goldenFlower")
    t.x = t.x + 140
    t.rotation = math.rad(90)
    -- t.cropLeft = 70

    local u = t:createChild("Sprite")
    u:setTexture("goldenFlower")
    u.x = u.x + 140
    -- u.cropLeft = 70

    self.props.s = s;

    local f = self:createChild("Sprite"):configure({
        texture = "freaker",
        animation = "running"
    })
    
    self.props.f = f;
    f.frame = 3

    local controlRight = Controls:scheme("right");
    controlRight:setKeys({ Key.Right, Key.D });

    local controlLeft = Controls:scheme("left");
    controlLeft:setKeys({ Key.Left, Key.A });

    local controlUp = Controls:scheme("up");
    controlUp:setKeys({ Key.Up, Key.W });

    local controlDown = Controls:scheme("down");
    controlDown:setKeys({ Key.Down, Key.S });

    self.camera:startFollow(f, 0)
end

scene.onUpdate = function(self, deltaTime)
    -- self.props.s:rotate(2 * math.pi * deltaTime * 0.25)
    
    local speed = 200;

    if Controls:isDown("right") then
        self.props.f:move(speed*deltaTime, 0)
    elseif Controls:isDown("left") then
        self.props.f:move(-speed*deltaTime, 0)
    end
    if Controls:isDown("up") then
        self.props.f:move(0, -speed*deltaTime)
    elseif Controls:isDown("down") then
        self.props.f:move(0, speed*deltaTime)
    end
end

return scene;