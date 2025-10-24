Nodes:define("DialogueBox", "NineSlice", {
    texture = "speechBox",

    marginLeft = 10,
    marginRight = 4,
    marginTop = 4,
    marginBottom = 8,

    maxWidth = 128,
    maxHeight = 128,
    width = 0,
    height = 0,

    visible = false,

    progressText = function(self)
        return self.input.pointer.justPressed or Controls:justPressed("confirm")
    end,

    onCreate = function(self)
        self.get.dialogueStack = {}
        self.get.isOpen = false

        self.get.root = self:createChild("Group", {
            onUpdate = function(root)
                root.x = -self.width*self.origin.x
                root.y = -self.height*self.origin.y
            end
        })

        self.get.text = self.get.root:createChild("Text", {
            font = "defaultFont",
            origin = 0,
            x = 4,
            y = 4,

            set = function(txt, str, onComplete)
                txt.text = str
                txt.wrapWidth = self.width - 8 -- Wrap text to width of box.
                txt.progress = 0
                txt:autoProgress({
                    rate = 40,

                    skipCondition = function()
                        -- Stops progressing the text, and skip to end.
                        return Controls:justPressed("confirm") or self.input.pointer.justPressed
                    end,

                    onComplete = function()
                        self:createChild("Action", {
                            onAct = function(actor, action)
                                if self.func:progressText() then
                                    if onComplete then
                                        onComplete()
                                    end
                                    action:complete()
                                end
                            end
                        })
                    end
                })
            end
        })
    end,

    open = function(self, onComplete)
        if self.get.isOpen then
            if onComplete then
                onComplete()
            end
            return
        end

        self.visible = true
        self.get.isOpen = true

        -- Tween opens the box.
        self.tween:to({
            width = 96,
            height = 64,
            ease = Ease.BackOut,
            duration = 0.2,
            onComplete = onComplete
        })
    end,

    close = function(self, onComplete)
        self.get.text.visible = false
        self.get.isOpen = false

        -- Tween closes the box.
        self.tween:to({
            width = 0,
            height = 0,
            ease = Ease.BackIn,
            duration = 0.2,
            onComplete = function()
                self.visible = false
                if onComplete then
                    onComplete()
                end
            end
        })
    end,

    say = function(self, str)
        table.insert(self.get.dialogueStack, str)
    end,

    execute = function(self)
        if #self.get.dialogueStack > 0 then
            local str = table.remove(self.get.dialogueStack, 1)
            

            -- Recursive set of callbacks going through the stack of dialogue strings.
            self.func:open(function()
                self.get.text.func:set(str, function()
                    self.func:execute()
                end)
            end)
        else
            self.func:close()
        end
    end
})