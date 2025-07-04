for i = 1, #Creator.worlds do
    local world = Creator.worlds[i]
    world.alwaysOnTop = true
    world:wait(0.1):next(function()
        world.alwaysOnTop = false
    end)
end