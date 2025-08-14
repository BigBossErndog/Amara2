Settings = {}

if System:exists("files/settings.json") then
    Settings = System:readJSON("files/settings.json")
end
if not Settings then
    Settings = {}
end 

Scripts:run("initialSetup/CheckCodeEditors.lua")

if System:VSBuildToolsInstalled() then
    Settings.vsBuildToolsInstalled = true
end

Settings.setupDone = true

System:writeFile("files/settings.json", Settings)