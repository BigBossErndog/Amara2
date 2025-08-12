local props = Game.argtable["-props"]

if props.iconDest then
    System:remove(props.iconDest)
end
if props.resFile then
    System:remove(props.resFile)
end
if props.resOutputFile then
    System:remove(props.resOutputFile)
end

System:copy(
    System:join(props.projectPath, "lua_scripts"),
    System:join(props.projectPath, "build", "windows", "lua_scripts")
)
System:copy(
    System:join(props.projectPath, "assets"),
    System:join(props.projectPath, "build", "windows", "assets")
)
if System:exists(System:join(props.projectPath, "files")) then
    System:copy(
        System:join(props.projectPath, "files"),
        System:join(props.projectPath, "build", "windows", "files")
    )
end