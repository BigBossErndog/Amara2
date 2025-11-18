-- This example shows you usage of a shader together with a uniform variable you can manipulate.

Nodes:load("Slider") -- Loads the Slider.lua file.

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
            vertex = "defaultVertex", -- Using the default vertex shader provided by the engine.

            -- 'defaultFragment' also exists.

            fragment = "shaders/boxBlur.frag", -- Loads the boxBlur.frag asset file. This is an OpenGL fragment shader.

            -- Box blur is a simple blur effect that averages the colors of neighboring pixels.
            -- More complicated blur effects exist, such as Gaussian blur.

            blurRadius = 5.0 -- Unrecognized key words are treated as uniform variables in the shader.
        })
    end,

    onCreate = function(self)
        -- Using a Container to crop the image to the left of the screen.
        self.get.leftContainer = self:createChild("Container", {
            origin = 0,
            x = self.world.view.left,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height
        })

        -- The image we will be blurring.
        self.get.starryNight = self.get.leftContainer:createChild("Sprite", {
            texture = "starryNight",
            x = self.get.leftContainer.right
        })

        -- A ShaderContainer will apply a given shader on its contents.
        self.get.rightContainer = self:createChild("ShaderContainer", {
            origin = 0,
            x = 0,
            y = self.world.view.top,
            width = self.world.view.width/2,
            height = self.world.view.height,
            shaderPass = "boxBlur",
            -- shaderPasses = { "boxBlur", "sepia", "wavy" } -- Multiple shader passes can be applied in sequence.
        })

        -- Using a copy node to render a duplicate of the starry night sprite.
        -- Added to the ShaderContainer as a child.
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