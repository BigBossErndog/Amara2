Settings = System:readJSON("files/settings.json")

Scripts:run("initialSetup/CheckCodeEditors")

System:writeFile("files/settings.json", Settings)