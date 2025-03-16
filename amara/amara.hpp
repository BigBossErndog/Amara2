#ifndef AMARA_STD
#define AMARA_STD
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <string>
    #include <iostream>
    #include <fstream>
    #include <streambuf>
    #include <time.h>
    #include <limits>
    #include <bitset>
    #include <map>
    #include <unordered_map>
    #include <vector>
    #include <deque>
    #include <list>
    #include <algorithm>
    #include <functional>
    #include <math.h>
    #include <regex>
    #include <random>
    #include <iomanip>
    #include <filesystem>
    #include <cstdlib>
    #include <cstdarg>
    #include <sstream>
    #include <nlohmann/json.hpp>
    #include <MurmurHash3.cpp>

    #if defined(_WIN32)
        #include <windows.h>
    #elif defined(__linux__)
    #elif defined(__ANDROID__)
    #elif defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE
        #else
        #endif
    #else
    #endif

    #define LUA_IMPLEMENTATION
    #include <lua_single.h>
    #include <sol/sol.hpp>
#endif

#ifndef AMARA_SDL
#define AMARA_SDL
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_ttf.h>
    #include <SDL_mixer.h>
    #include <SDL_FontCache.c>
    #ifdef AMARA_WEB
        #include <SDL_net.h>
    #endif
    #ifdef AMARA_NOVIDEO
        #include <sdlogv.h>
    #endif
#endif

#ifndef AMARA_DEF
#define AMARA_DEF
    #include "amara2_utility.cpp"
    #include "amara2_geometry.cpp"

    #include "amara2_properties.cpp"
    #include "amara2_luaUtlity.cpp"

    #include "amara2_gameManager.cpp"

    #include "amara2_fileManager.cpp"
    
    #include "amara2_messages.cpp"
    #include "amara2_messageBox.cpp"

    #include "amara2_script.cpp"
    #include "amara2_scriptFactory.cpp"

    #include "amara2_entity.cpp"
    #include "amara2_scene.cpp"

    #include "amara2_entityFactory.cpp"

    #include "amara2_world.cpp"
    #include "amara2_creator.cpp"
#endif