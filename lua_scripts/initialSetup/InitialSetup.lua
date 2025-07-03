Settings = {}

if System:exists("data/settings.json") then
    Settings = System:readJSON("data/settings.json")
end

if Game.platform == "windows" then
    if not System:VSBuildToolsInstalled() then
        Settings.vsBuildToolsNotInstalled = true
    end
end

Scripts:run("initialSetup/CheckCodeEditors.lua")

Settings.setupDone = true

System:writeFile("data/settings.json", Settings)