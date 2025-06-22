Localize = {
    register = function(lib, key, word)
        if not lib.words then
            lib.words = {}
        end
        lib.words[key] = word
    end,
    registerJSON = function(lib, json)
        for key, word in pairs(json) do
            lib:register(key, word)
        end
    end,
    get = function(lib, key)
        if not lib.words then
            return key
        end
        if not lib.words[key] then
            return key
        end
        return lib.words[key]
    end,
    has = function(lib, key)
        if not lib.words then
            return false
        end
        if lib.words[key] then
            return true
        end
        return false
    end
}