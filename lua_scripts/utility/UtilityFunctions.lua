function quote_if_needed(path)
    if string.find(path, " ") then
        return '"' .. path .. '"'
    else
        return path
    end
end

function fix_path(path, allow_backslashes)
    if not allow_backslashes then
        return quote_if_needed(string.gsub(path, "\\", "/"))
    else
        return quote_if_needed(path)
    end
end

function escape_double_quotes(str)
    return (str:gsub('"', '\\"'))
end