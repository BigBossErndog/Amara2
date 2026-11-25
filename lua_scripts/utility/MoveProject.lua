local oldProjectDirectory = Game.args["oldProjectDirectory"]
local newProjectDirectory = Game.args["newProjectDirectory"]
local projectName = Game.args["projectName"]

local oldProjectData = System:readJSON(string.joinpath(oldProjectDirectory, "project.json"))

System:copy(oldProjectDirectory, newProjectDirectory)

if not oldProjectData.exampleProject then
    System:remove(oldProjectDirectory)
end

local projectData = System:readJSON(string.joinpath(newProjectDirectory, "project.json"))
if projectData["executable-name"] == projectData["project-name"] or not projectData["executable-name"] then
    projectData["executable-name"] = projectName
end
projectData["project-name"] = projectName

projectData.uninitiated = true

if projectData.exampleProject then
    projectData.exampleProject = nil
end

System:writeFile(string.joinpath(newProjectDirectory, "project.json"), projectData)