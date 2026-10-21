#pragma once
#include <string>

#if defined(_MSC_VER)
    #ifdef AMARA_ENGINE_TOOLS
        #pragma detect_mismatch("amara2_variant", "engine")
    #else
        #pragma detect_mismatch("amara2_variant", "plain")
    #endif

    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "winmm.lib")
    #pragma comment(lib, "imm32.lib")
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "oleaut32.lib")
    #pragma comment(lib, "version.lib")
    #pragma comment(lib, "advapi32.lib")
    #pragma comment(lib, "setupapi.lib")
    #ifdef AMARA_OPENGL
        #pragma comment(lib, "opengl32.lib")
    #endif

    #ifndef AMARA_IMPLEMENTATION
        #ifdef AMARA_ENGINE_TOOLS
            #pragma comment(lib, "amara2-engine.lib")
        #else
            #pragma comment(lib, "amara2.lib")
        #endif
    #endif
#endif

int amara_run(int argc, char** argv, const char* encryption_key = nullptr, bool encrypt_output = false, const std::string& build_args_str = "");

#ifndef AMARA_IMPLEMENTATION
    #include <stdio.h>
    #include <stdlib.h>

    #if defined(_WIN32)
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <windows.h>
    #endif

    #ifndef AMARA_STR
        #define AMARA_STR(x) #x
        #define AMARA_STRINGIFY(x) AMARA_STR(x)
    #endif
#else
    #ifndef AMARA_STD
        #define AMARA_STD
        #include <stdio.h>
        #include <stdlib.h>
        
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
        #include <cstdint>
        #include <atomic>
        #include <thread>
        #include <mutex>
        #include <type_traits>
        #include <chrono>
        #include <memory>

        #if defined(__ANDROID__)
        #include <jni.h>
        #endif
    #endif

    #ifndef AMARA_EXTLIBS
        #define AMARA_EXTLIBS
        #include <nlohmann/json.hpp>

        #define STB_IMAGE_IMPLEMENTATION
        #include <stb_image.h>
        #define STB_IMAGE_RESIZE_IMPLEMENTATION
        #include <stb_image_resize2.h>
        #define STB_IMAGE_WRITE_IMPLEMENTATION
        #include <stb_image_write.h>

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

        #if defined(AMARA_INCLUDE_BOX2D)
        #include <box2d/box2d.h>
        #endif

        #if defined(_WIN32)
            #define AMARA_DESKTOP
            #include <windows.h>
            #include <shellapi.h>
            #include <winhttp.h>
            #pragma comment(lib, "winhttp.lib")
            #undef min
            #undef max
        #elif defined(__ANDROID__)
        #elif defined(__linux__)
            #define AMARA_DESKTOP
            #include <SDL_syswm.h>
            #include <sys/socket.h>
            #include <netinet/in.h>
            #include <unistd.h>
            #include <X11/Xlib.h>
            #include <X11/extensions/shape.h>
        #elif defined(__APPLE__)
            #include <SDL_syswm.h>
            #include <TargetConditionals.h>
            #include <CoreGraphics/CoreGraphics.h>
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

        #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
        #include "miniz.h"
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
            #include <emscripten.h>
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
        #ifndef AMARA_STR
            #define AMARA_STR(x) #x
            #define AMARA_STRINGIFY(x) AMARA_STR(x)
        #endif

        #ifndef M_PI
            #define M_PI 3.14159265358979323846264338327950288
        #endif

        #include "utility/amara2_enums.cpp"
        #include "utility/amara2_string.cpp"
        #include "utility/amara2_encryption.cpp"
        #include "utility/amara2_utility.cpp"
        #include "utility/amara2_jsonUtility.cpp"
        #include "utility/amara2_color.cpp"
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
        #include "utility/amara2_luaUtility.cpp"
        #include "utility/amara2_csvUtility.cpp"

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

        #include "nodes/amara2_syncDirectory.cpp"

        #include "actions/amara2_action.cpp"
        #include "actions/amara2_waitAction.cpp"
        #include "actions/amara2_tween.cpp"
        #include "actions/amara2_stateMachine.cpp"
        #include "actions/amara2_delayNode.cpp"
        #include "actions/amara2_repeatAction.cpp"
        #include "actions/amara2_processNode.cpp"

        #include "actions/amara2_browseDirectory.cpp"
        #include "actions/amara2_importZip.cpp"
        #include "actions/amara2_exportZip.cpp"

        #include "actions/amara2_textInput.cpp"

        #include "actions/amara2_loader.cpp"
        
        #include "actions/amara2_autoProgress.cpp"
        
        #include "nodes/amara2_sprite.cpp"
        #include "nodes/amara2_repeatSprite.cpp"
        #include "nodes/amara2_particleEmitter.cpp"
        #include "actions/amara2_animation.cpp"

        #include "nodes/amara2_fillRect.cpp"
        #include "nodes/amara2_fillCircle.cpp"

        #include "nodes/amara2_container.cpp"

        #include "nodes/amara2_textureContainer.cpp"
        #include "nodes/amara2_textureLayer.cpp"
        
        #include "nodes/amara2_quadSprite.cpp"
        #include "nodes/amara2_quadContainer.cpp"

        #ifdef AMARA_OPENGL
        #include "nodes/amara2_shaderContainer.cpp"
        #include "nodes/amara2_shaderLayer.cpp"
        #include "nodes/amara2_shaderCamera.cpp"
        #endif
        
        #include "nodes/tilemap/amara2_tilemapLayer.cpp"
        #include "nodes/tilemap/amara2_tilemap.cpp"

        #include "nodes/amara2_nineSlice.cpp"

        #include "nodes/amara2_pathFinder.cpp"

        #include "physics/amara2_collider.cpp"

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

    int amara_run(int argc, char** argv, const char* encryption_key, bool encrypt_output, const std::string& build_args_str) {
        Amara::Creator creator(argc, argv);

        if (encryption_key != nullptr && encryption_key[0] != '\0') {
            creator.encryption_key = encryption_key;
            if (encrypt_output) creator.encrypt_output = true;
            creator.setEncryption();
        }

        if (!build_args_str.empty()) {
            creator.build_args = Amara::string_to_json(build_args_str);
        }
        
        if (creator.starting_scripts.size() > 0) {
            return creator.startCreation();
        }

        std::string demiurgicPath = creator.system.getRelativePath("demiurgic.world");
        if (creator.system.exists(demiurgicPath)) {
            return creator.startCreation(demiurgicPath);
        }

        std::string indexPath = creator.system.getScriptPath("index");
        if (creator.system.exists(indexPath)) {
            return creator.startCreation(indexPath);
        }
        return 1;
    }
 #endif