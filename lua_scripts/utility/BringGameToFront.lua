for i = 1, #Creator.worlds do
    local world = Creator.worlds[i]
    local prev = world.alwaysOnTop
    world.alwaysOnTop = true
    world.alwaysOnTop = prev
end