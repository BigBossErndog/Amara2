-- Welcome to your new world

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Particles"
    },
    
    onPreload = function(self)
        self.load:spritesheet("particles", "particles.png", 8, 8)
    end,

    onCreate = function(self)
        self.get.myParticles = self:createChild("ParticleEmitter", {
            particles = {
                texture = "particles", -- Use an image or spritesheet

                spawning = true, -- Set spawning state.

                poolSize = 2000, -- Max amount that can be shown at once, if omitted default is 128.
                spawnRate = 100, -- How many to spawn per second.
                lifeTime = 1, -- Set how long a particle will exist for.

                tint = "white",
                endTint = "#a34dff",

                velocityX = {
                    min = -100,
                    max = 100
                }, -- Starting velocity randomly between -100 and 100

                velocityY = {
                    min = -200,
                    max = 0
                }, -- Throws particles into the air.

                accelerationY = 200, -- Applies some gravity

                frame = { 1, 2 }, -- Choose random frame between 1 and 2

                angularVelocity = {
                    min = -math.pi,
                    max = math.pi
                }, -- Spins the particle at a rate randomly between -pi and pi.

                alpha = 1,
                endAlpha = 0 -- Fade the particle out.
            }
        })
    end,

    onUpdate = function(self, deltaTime)
        if Keyboard:justPressed(Key.Space) or self.input.pointer.justPressed then
            self.get.myParticles.spawning = not self.get.myParticles.spawning -- toggle spawning state on/off
        end
    end
})