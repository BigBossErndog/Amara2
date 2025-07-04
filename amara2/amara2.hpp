#pragma once

#ifndef AMARA_STD
#define AMARA_STD
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    
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
    #include <mutex>
#endif

#ifndef AMARA_EXTLIBS
#define AMARA_EXTLIBS
    #include <nlohmann/json.hpp>

    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>

    #define STB_TRUETYPE_IMPLEMENTATION
    #include <stb_truetype.h>

    #include <stb_vorbis.c>
    #ifdef R
    #undef R
    #endif
    #ifdef L
    #undef L
    #endif
    #ifdef C
    #undef C
    #endif

    #define MINIMP3_IMPLEMENTATION
    #define MINIMP3_ENABLE_MP3_FLOAT
    #include <minimp3_ex.h>

    #include <tinyxml2.cpp>

    #ifdef AMARA_INCLUDE_BOX2D
    #include <box2d/box2d.h>
    #endif

    #if defined(_WIN32)
        #define AMARA_DESKTOP
        #include <windows.h>
    #elif defined(__linux__)
        #define AMARA_DESKTOP
        #include <SDL_syswm.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <unistd.h>
        #include <X11/Xlib.h>
        #include <X11/extensions/shape.h>
    #elif defined(__ANDROID__)
    #elif defined(__APPLE__)
        #include <SDL_syswm.h>
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE
        #else
            #define AMARA_DESKTOP
        #endif

        struct NSView;
        struct NSWindow;
    #else
    #endif

    #ifdef AMARA_DESKTOP
        #include <portable-file-dialogs.h>
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

    #include "utility/amara2_enums.cpp"
    #include "utility/amara2_string.cpp"
    #include "utility/amara2_jsonUtility.cpp"
    #include "utility/amara2_utility.cpp"
    #include "utility/amara2_color.cpp"
    #include "utility/amara2_encryption.cpp"
    #include "utility/amara2_easing.cpp"

    #ifdef AMARA_OPENGL
        #include "rendering/amara2_openGL.cpp"
    #endif 
    
    #include "geometry/amara2_vectors.cpp"
    #include "geometry/amara2_shapes.cpp"
    #include "geometry/amara2_geometry.cpp"

    #include "rendering/amara2_gpuHandler.cpp"

    #include "managers/amara2_gameProps.cpp"
    
    #include "rendering/amara2_renderUtility.cpp"
    #include "utility/amara2_luaUtlity.cpp"

    #include "input/amara2_button.cpp"
    #include "input/amara2_keyboard.cpp"
    #include "input/amara2_gamepad.cpp"
    #include "input/amara2_pointer.cpp"
    #include "input/amara2_mouse.cpp"
    #include "input/amara2_touchHandler.cpp"
    #include "input/amara2_controlScheme.cpp"
    #include "input/amara2_controlManager.cpp"

    #include "input/amara2_inputManager.cpp"

    #include "managers/amara2_gameManager.cpp"

    #include "managers/amara2_systemManager.cpp"
    
    #include "assets/amara2_asset.cpp"
    #include "assets/amara2_imageAsset.cpp"
    #include "assets/amara2_fontAsset.cpp"
    #include "assets/amara2_tmxTilemapAsset.cpp"
    #include "assets/amara2_audioAsset.cpp"

    #ifdef AMARA_OPENGL
    #include "rendering/shaders/amara2_shaderProgram.cpp"
    #endif
    
    #include "managers/amara2_assetManager.cpp"
    #include "managers/amara2_shaderManager.cpp"
    
    #include "rendering/amara2_renderBatch.cpp"

    #include "managers/amara2_messages.cpp"
    #include "managers/amara2_messageBox.cpp"

    #include "managers/amara2_functionManager.cpp"

    #include "input/amara2_nodeInput.cpp"
    
    #include "factories/amara2_scriptFactory.cpp"
    #include "factories/amara2_animationFactory.cpp"

    #include "nodes/amara2_node.cpp"

    #include "nodes/amara2_group.cpp"
    #include "nodes/amara2_copyNode.cpp"
    #include "nodes/amara2_nodePool.cpp"

    #ifdef AMARA_WEB_SERVER
    #include "nodes/amara2_webServer.cpp"
    #endif

    #include "nodes/amara2_text.cpp"

    #include "nodes/amara2_camera.cpp"
    
    #include "nodes/amara2_scene.cpp"

    #include "actions/amara2_action.cpp"
    #include "actions/amara2_waitAction.cpp"
    #include "actions/amara2_tween.cpp"
    #include "actions/amara2_stateMachine.cpp"
    #include "actions/amara2_delayNode.cpp"
    #include "actions/amara2_periodicAction.cpp"
    #include "actions/amara2_processNode.cpp"

    #include "actions/amara2_textInput.cpp"

    #include "actions/amara2_loader.cpp"
    
    #include "actions/amara2_autoProgress.cpp"
    
    #include "nodes/amara2_sprite.cpp"
    #include "actions/amara2_animation.cpp"

    #include "nodes/amara2_fillRect.cpp"

    #include "nodes/amara2_container.cpp"

    #include "nodes/amara2_textureContainer.cpp"
    #include "nodes/amara2_shaderContainer.cpp"
    
    #include "nodes/tilemap/amara2_tilemapLayer.cpp"
    #include "nodes/tilemap/amara2_tilemap.cpp"

    #include "nodes/amara2_nineSlice.cpp"

    #include "nodes/amara2_pathFinder.cpp"

    #include "audio/amara2_audio.cpp"
    #include "audio/amara2_audiogroup.cpp"
    #include "audio/amara2_audioPool.cpp"
    #include "audio/amara2_audioMaster.cpp"

    #include "transitions/amara2_transition.cpp"
    #include "transitions/amara2_fillTransition.cpp"
    
    #include "nodes/amara2_world.cpp"

    #ifdef AMARA_PLUGINS
    #include <amara2_plugins.cpp>
    #endif
    
    #include "factories/amara2_nodeFactory.cpp"

    #include "managers/amara2_garbage.cpp"

    #include "managers/amara2_eventHandler.cpp"

    #include "garden/amara2_demiurge.cpp"
    #include "garden/amara2_creator.cpp"
#endif