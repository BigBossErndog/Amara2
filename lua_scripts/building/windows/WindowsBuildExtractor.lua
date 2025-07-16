print("HELLO!")
local argtable = Game.argtable

local buildModulePath
if argtable["-buildmodule"] then
    buildModulePath = argtable["-buildmodule"]
end

if not buildModulePath or not System:exists(buildModulePath) then
    fatal_error("Error: Build module not found at \"", buildModulePath, "\"")
end

if not System:exists("build_modules") then
    System:createDirectory("build_modules")
end

print("Extracting module: \"", System:getFileName(buildModulePath), "\"")
System:unzip(buildModulePath, "build_modules")