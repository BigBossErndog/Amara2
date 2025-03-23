#ifndef AMARA_STD
#define AMARA_STD
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <string>
    #include <iostream>
    #include <fstream>
    #include <sstream>
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
    #include <atomic>
    #include <thread>
    #include <nlohmann/json.hpp>

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
    #include <SDL3/SDL.h>
    #ifdef __EMSCRIPTEN__
        #include <emscripten/fetch.h>
    #endif
    #ifdef AMARA_WEB
        #include <SDL_net.h>
    #endif
    #ifdef AMARA_NOVIDEO
        #include <sdlogv.h>
    #endif
#endif

#ifndef AMARA_DEF
#define AMARA_DEF
    #include "utility/amara2_utility.cpp"
    #include "utility/amara2_easing.cpp"

    #include "geometry/amara2_vectors.cpp"
    #include "geometry/amara2_shapes.cpp"
    #include "geometry/amara2_geometry.cpp"

    #include "managers/amara2_props.cpp"

    #include "utility/amara2_luaUtlity.cpp"

    #include "managers/amara2_gameManager.cpp"

    #include "managers/amara2_fileManager.cpp"
    
    #include "managers/amara2_messages.cpp"
    #include "managers/amara2_messageBox.cpp"
    
    #include "factories/amara2_scriptFactory.cpp"

    #include "entities/amara2_entity.cpp"

    #ifdef AMARA_WEB_SERVER
        #include "entities/amara2_webServer.cpp"
    #endif

    #include "entities/amara2_camera.cpp"
    
    #include "entities/amara2_scene.cpp"

    #include "entities/amara2_action.cpp"
    #include "entities/amara2_tween.cpp"
    #include "entities/amara2_stateMachine.cpp"
    
    #include "entities/amara2_world.cpp"

    #include "factories/amara2_entityFactory.cpp"

    #include "managers/amara2_garbage.cpp"

    #include "managers/amara2_inputManager.cpp"

    #include "garden/amara2_demiurge.cpp"
    #include "garden/amara2_creator.cpp"
#endif