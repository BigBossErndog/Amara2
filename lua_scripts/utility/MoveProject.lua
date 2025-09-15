local oldProjectDirectory = Game.argtable["-oldProjectDirectory"]
local newProjectDirectory = Game.argtable["-newProjectDirectory"]
local projectName = Game.argtable["-projectName"]

System:copy(oldProjectDirectory, newProjectDirectory)

System:remove(oldProjectDirectory)

local projectData = System:readJSON(System:join(newProjectDirectory, "project.json"))
if projectData["executable-name"] == projectData["project-name"] or not projectData["executable-name"] then
    projectData["executable-name"] = projectName
end
projectData["project-name"] = projectName

projectData.uninitiated = true

System:writeFile(System:join(newProjectDirectory, "project.json"), projectData)