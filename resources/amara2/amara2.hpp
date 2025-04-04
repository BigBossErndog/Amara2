#pragma once

#ifndef AMARA_STD
#define AMARA_STD
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <string>
    #include <codecvt>
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <streambuf>
    #include <time.h>
    #include <limits>
    #include <bitset>
    #include <map>
    #include <unordered_map>
    #include <unordered_set>
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
#endif

#ifndef AMARA_EXTLIBS
#define AMARA_EXTLIBS
    #include <nlohmann/json.hpp>

    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
    

    #define STB_TRUETYPE_IMPLEMENTATION
    #include <stb_truetype.h>

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
    #define SOL_ALL_SAFETIES_ON 1
    
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
    #ifndef M_PI
        #define M_PI 3.14159265358979323846264338327950288
    #endif

    #include "utility/amara2_string.cpp"
    #include "utility/amara2_utility.cpp"
    #include "utility/amara2_easing.cpp"

    #ifdef AMARA_OPENGL
        #include "rendering/amara2_openGL.cpp"
    #endif 
    
    #include "geometry/amara2_vectors.cpp"
    #include "geometry/amara2_shapes.cpp"
    #include "geometry/amara2_geometry.cpp"

    #include "rendering/amara2_renderUtility.cpp"
    #include "rendering/amara2_gpuHandler.cpp"

    #include "managers/amara2_props.cpp"

    #include "utility/amara2_luaUtlity.cpp"

    #include "input/button.cpp"
    #include "input/keyboard.cpp"
    #include "input/controlScheme.cpp"
    #include "input/controlManager.cpp"

    #include "managers/amara2_gameManager.cpp"

    #include "managers/amara2_fileManager.cpp"
    
    #include "assets/amara2_asset.cpp"
    #include "assets/amara2_imageAsset.cpp"
    #include "assets/amara2_fontAsset.cpp"

    #ifdef AMARA_OPENGL
    #include "rendering/amara2_shaderProgram.cpp"
    #endif
    
    #include "managers/amara2_assetManager.cpp"
    #include "managers/amara2_shaderManager.cpp"
    
    #include "rendering/amara2_renderBatch.cpp"

    #include "managers/amara2_messages.cpp"
    #include "managers/amara2_messageBox.cpp"
    
    #include "factories/amara2_scriptFactory.cpp"
    #include "factories/amara2_animationFactory.cpp"

    #include "nodes/amara2_node.cpp"

    #ifdef AMARA_WEB_SERVER
        #include "nodes/amara2_webServer.cpp"
    #endif

    #include "nodes/amara2_text.cpp"

    #include "nodes/amara2_camera.cpp"
    
    #include "nodes/amara2_scene.cpp"

    #include "actions/amara2_action.cpp"
    #include "actions/amara2_tween.cpp"
    #include "actions/amara2_stateMachine.cpp"
    #include "actions/amara2_loader.cpp"
    
    #include "nodes/amara2_sprite.cpp"
    #include "actions/amara2_animation.cpp"

    #include "audio/amara2_audio.cpp"
    #include "audio/amara2_audiogroup.cpp"
    #include "audio/amara2_audioMaster.cpp"
    
    #include "nodes/amara2_world.cpp"

    #include "factories/amara2_nodeFactory.cpp"

    #include "managers/amara2_garbage.cpp"

    #include "managers/amara2_inputManager.cpp"

    #include "garden/amara2_demiurge.cpp"
    #include "garden/amara2_creator.cpp"
#endif