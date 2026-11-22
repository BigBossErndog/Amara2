function quote_if_needed(path)
    if string.find(path, " ") then
        return '"' .. path .. '"'
    else
        return path
    end
end

function quote_bat_arg(s)
    s = s:gsub('(\\*)"', function(bs) return bs .. bs .. '\\"' end)
    s = s:gsub('(\\+)$', '%1%1')
    s = '"' .. s .. '"'

    s = s:gsub("%%", "%%%%")

    local out, in_quotes = {}, false
    for c in s:gmatch(".") do
        if c == '"' then
            in_quotes = not in_quotes
        elseif not in_quotes and c:find("[%^&|<>()]") then
            out[#out + 1] = "^"
        end
        out[#out + 1] = c
    end
    return table.concat(out)
end

function quote_bat_str(s)
    s = s:gsub('[\\"]', '\\%0')
    s = s:gsub("\r", "\\r")
    s = s:gsub("\n", "\\n")
    s = s:gsub("\t", "\\t")
    s = '"' .. s .. '"'
    
    return quote_bat_arg(s)
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