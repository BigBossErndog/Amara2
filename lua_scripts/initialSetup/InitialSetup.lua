Settings = {}

if System:exists("data/settings.json") then
    Settings = System:readJSON("data/settings.json")
end

Scripts:run("initialSetup/CheckCodeEditors.lua")

if System:VSBuildToolsInstalled() then
    Settings.vsBuildToolsInstalled = true
end

Settings.setupDone = true

System:writeFile("data/settings.json", Settings)