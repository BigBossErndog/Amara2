-- Shaders are useful for affecting the way your game is rendered to the screen.
-- Here are examples of various shaders you may want to use.

Nodes:define("ArtScene", "Scene", {
    onCreate = function(self)
        -- Replacing the main camera with a ShaderCamera, which allows us to use shaders.
        self.camera = self:createChild("ShaderCamera", {
            x = self.world.view.left,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height/2,
            origin = 0,

            shaderPass = "sepia"
        })

        -- Adding additional cameras.

        self:createChild("ShaderCamera", {
            x = 0,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height/2,
            origin = 0,
            
            shaderPass = "grayscale"
        })

        self:createChild("ShaderCamera", {
            x = self.world.view.left,
            y = 0,
            width = self.world.view.width/2,
            height = self.world.view.height/2,
            origin = 0,

            shaderPass = "bloom"
        })

        self:createChild("ShaderCamera", {
            x = 0,
            y = 0,
            width = self.world.view.width/2,
            height = self.world.view.height/2,
            origin = 0,

            shaderPass = "wavy"
        })


        -- Creating the image.

        self.get.artwork = self:createChild("Sprite", {
            texture = "artwork"
        })
    end
})

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,

        graphics = Graphics.OpenGL, -- We are using OpenGL shaders so make sure to set to OpenGL graphics.
        
        title = "Shaders - Various"
    },
    
    onPreload = function(self)
        self.load:image("artwork", "the_scream.png")

        -- The following fragment shaders change color.
        self.load:shaderProgram("sepia", {
            vertex = "defaultVertex",
            fragment = "shaders/sepia.frag"
        })
        self.load:shaderProgram("grayscale", {
            vertex = "defaultVertex",
            fragment = "shaders/grayscale.frag"
        })
        self.load:shaderProgram("bloom", {
            vertex = "defaultVertex",
            fragment = "shaders/bloom.frag"
        })

        -- The following vertex shader changes the position of rendered quads.
        self.load:shaderProgram("wavy", {
            vertex = "shaders/wavy.vert",
            fragment = "defaultFragment"
        })
    end,

    onCreate = function(self)
        self:createChild("ArtScene")
    end,

    onUpdate = function(self, deltaTime)

    end
})