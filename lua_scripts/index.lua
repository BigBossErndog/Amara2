-- game:uncapFPS()

-- local e = factory:create("Entity");
-- e.props.hello = function() log("hi") end

local w = Creator:createWorld()

-- game:setTargetFPS(60)

local e = w:createChild("Entity")
-- -- e.id = "acting entity"

-- w.onUpdate = function(self) log(math.floor(game.fps)) end
-- w.id = "hi"
-- e.onUpdate = function(self) log(world.x) end

-- print_metatable(w)

e.tween:from({
        x = 10
    }):to({
        x = 0,
        duration = 1,
        onUpdate = function(self)
            print(e.x)
        end,
        ease = Ease.SineOut
    }):to({
        x = 10,
        duration = 1,
        onUpdate = function(self)
            print(e.x)
        end
    }):whenDone(function(self)
        print("finished")
        self.world:destroy()
    end)

-- scripts:run("scripts/testScript")