Settings = {}

if System:exists("data/settings.json") then
    Settings = System:readJSON("data/settings.json")
end

Scripts:run("initialSetup/CheckCodeEditors.lua")

Settings.setupDone = true

System:writeFile("data/settings.json", Settings)