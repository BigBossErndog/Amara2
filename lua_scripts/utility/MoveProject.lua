local oldProjectDirectory = Game.argtable["oldProjectDirectory"]
local newProjectDirectory = Game.argtable["newProjectDirectory"]
local projectName = Game.argtable["projectName"]

local oldProjectData = System:readJSON(System:join(oldProjectDirectory, "project.json"))

System:copy(oldProjectDirectory, newProjectDirectory)

if not oldProjectData.exampleProject then
    System:remove(oldProjectDirectory)
end

local projectData = System:readJSON(System:join(newProjectDirectory, "project.json"))
if projectData["executable-name"] == projectData["project-name"] or not projectData["executable-name"] then
    projectData["executable-name"] = projectName
end
projectData["project-name"] = projectName

projectData.uninitiated = true

if projectData.exampleProject then
    projectData.exampleProject = nil
end

System:writeFile(System:join(newProjectDirectory, "project.json"), projectData)