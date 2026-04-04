Scripts:include("DialogueBox.lua")

Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Dialogue Box"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        self.load:image("speechBox", "speechBox.png")
    end,

    onCreate = function(self)
        local confirmControl = Controls:scheme("confirm")
        confirmControl:setKeys(Key.Space, Key.Enter, Key.Z)
        
        self.get.dialogueBox = self:createChild("DialogueBox")
        self.get.dialogueBox.func:say("Top of the morning to you, good sir!")
        self.get.dialogueBox.func:say("Lovely day, innit gov'ner?")
        self.get.dialogueBox.func:say("Hasta luego, mi amigo.")

        self.get.dialogueBox.func:execute()
    end,

    onUpdate = function(self, deltaTime)
        
    end
})