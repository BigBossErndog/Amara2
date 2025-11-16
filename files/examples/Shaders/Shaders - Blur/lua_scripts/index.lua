Nodes:load("Slider")

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,

        graphics = Graphics.OpenGL, -- You must use OpenGL graphics to use OpenGL shaders.
        
        title = "Shaders - Blur"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
        self.load:image("starryNight", "starryNight.png")

        -- Load the box blur shader program
        -- A shader program must at least have a vertex and fragment shader.
        -- Other shader types such as geometry shaders are optional.
        self.load:shaderProgram("boxBlur", {
            vertex = "defaultVertex",
            fragment = "shaders/boxBlur.frag",

            blurRadius = 5.0 -- Unrecognized key words are treated as uniform variables in the shader.
        })
    end,

    onCreate = function(self)
        self.get.leftContainer = self:createChild("Container", {
            origin = 0,
            x = self.world.view.left,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height
        })

        self.get.starryNight = self.get.leftContainer:createChild("Sprite", {
            texture = "starryNight",
            x = self.get.leftContainer.right
        })

        self.get.rightContainer = self:createChild("ShaderContainer", {
            origin = 0,
            x = 0,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height,
            shaderPass = "boxBlur"
        })

        -- Using a copy node to render a duplicate of the starry night sprite
        self.get.starryNightCopy = self.get.rightContainer:createChild("CopyNode", {
            target = self.get.starryNight,
            x = -self.get.leftContainer.width/2 - self.get.rightContainer.width/2
        })

        self.get.slider = self:createChild("Slider", {
            x = 0,
            y = 75,
            value = 0.5,
            text = "Blur Amount",
            onChange = function(value)
                -- Get the shader program and set the uniform variable "blurRadius"
                self.assets:getShaderProgram("boxBlur"):setUniform("blurRadius", math.floor(value * 10))
            end
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})