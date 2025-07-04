for i = 1, #Creator.worlds do
    local world = Creator.worlds[i]
    local prev = world.alwaysOnTop
    world.alwaysOnTop = true
    world:wait(0.1):next(function()
        world.alwaysOnTop = prev
    end)
end