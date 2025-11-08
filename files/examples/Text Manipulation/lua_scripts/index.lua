Creator:createWorld({
    window = {
        width = 640,
        height = 360,
        virtualWidth = 320,
        virtualHeight = 180,

        backgroundColor = "#294d6a",

        screenMode = ScreenMode.Windowed,
        
        title = "Text Manipulation"
    },
    
    onPreload = function(self)
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)
    end,

    onCreate = function(self)
        Assets:setDefaultFont("defaultFont")

        self:createChild("Text", {
            text = "This text is ${red}red${end}.",
            y = -32
        })

        self:createChild("Text", {
            text = "This text is ${wavy}wavy${wavy}.",
            manipulators = {
                wavy = function(index, time, char)
                    return {
                        offsetY = math.sin(time * 5 + index * 0.8) * 0.8
                    }
                end
            }
        })

        self:createChild("Text", {
            text = "This text is ${rainbow}rainbow${end}.",
            y = 32,
            manipulators = {
                rainbow = function(index, time, char)
                    local hue = (index * 0.08 + time * 0.2) % 1
                    local s = 0.9
                    local v = 0.95

                    local c = v * s
                    local h6 = hue * 6
                    local x = c * (1 - math.abs((h6 % 2) - 1))
                    local r1, g1, b1
                    if h6 < 1 then
                        r1, g1, b1 = c, x, 0
                    elseif h6 < 2 then
                        r1, g1, b1 = x, c, 0
                    elseif h6 < 3 then
                        r1, g1, b1 = 0, c, x
                    elseif h6 < 4 then
                        r1, g1, b1 = 0, x, c
                    elseif h6 < 5 then
                        r1, g1, b1 = x, 0, c
                    else
                        r1, g1, b1 = c, 0, x
                    end
                    local m = v - c
                    local r = math.floor((r1 + m) * 255 + 0.5)
                    local g = math.floor((g1 + m) * 255 + 0.5)
                    local b = math.floor((b1 + m) * 255 + 0.5)

                    return {
                        color = Color.new(r, g, b) * Color.new(240, 240, 240)
                    }
                end
            }
        })
    end,

    onUpdate = function(self, deltaTime)

    end
})