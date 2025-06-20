Localize = {
    register = function(lib, key, word)
        if not lib.words then
            lib.words = {}
        end
        lib.words[key] = word
    end,
    get = function(lib, key)
        if not lib.words then
            return key
        end
        return lib.words[key]
    end
}