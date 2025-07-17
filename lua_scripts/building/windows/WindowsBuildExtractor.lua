local argtable = Game.argtable

local buildModulePath
if argtable["-buildmodule"] then
    buildModulePath = argtable["-buildmodule"]
end

if not buildModulePath or not System:exists(buildModulePath) then
    fatal_error("Error: Build module not found at \"", buildModulePath, "\"")
end

print("Extracting module: \"" .. System:getFileName(buildModulePath) .. "\"")
print("Note: Please wait. This may take a while.")

System:unzip(buildModulePath, "build_modules")

print("Extraction complete. You are now ready to build your projects.")