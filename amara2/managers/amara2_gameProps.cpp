namespace Amara {
    class Node;
    class World;
    class Scene;

    class GameManager;

    class NodeFactory;
    class ScriptFactory;
    class AnimationFactory;

    class SystemManager;
    class InputManager;
    class AssetManager;
    class ShaderManager;

    class RenderBatch;

    class Audio;
    class AudioMaster;

    class MessageQueue;
    class GarbageCollector;

    class KeyboardManager;
    class GamepadManager;
    class ControlManager;
    class Pointer;

    class GPUHandler;
    class Asset;

    #ifdef AMARA_OPENGL
    class ShaderProgram;
    #endif

    struct AudioData {
        Amara::Audio* audio = nullptr;
        SDL_AudioDeviceID device = 0;
        SDL_AudioSpec spec;

        float volume = 1;

        float panning = 0;
        
        void reset() {
            AudioData new_data;
            new_data.device = device;

            *this = new_data;
        }
    };

    struct PassOnProps {
        float alpha = 1;
        
        float rotation = 0;
        Vector3 anchor = { 0, 0, 0 };
        Vector2 scale = { 1, 1 };
        
        Vector2 scroll = { 0, 0 };
        Vector2 zoom = { 1, 1 };

        Vector2 window_zoom = { 1, 1 };

        bool insideTextureContainer = false;
        
        void reset() {
            PassOnProps new_props;
            new_props.window_zoom = window_zoom;

            *this = new_props;
        }
    };

    class GameProps {
    public:
        sol::state lua;
        int lua_stack_size = 1000000;

        #ifdef AMARA_TEST_PATH
        std::string context_path = AMARA_TEST_PATH;
        #else
        std::string context_path = "./";
        #endif
        std::string lua_script_path = "./lua_scripts";
        std::string assets_path = "./assets";

        bool integrate_new_windows = false;
        bool transparent_window = false;

        double deltaTime = 1;
        
        Amara::World* render_origin = nullptr;
        Rectangle master_viewport;
        Rectangle display;

        PassOnProps passOn;
        AudioData audioData;
        std::vector<AudioData> audio_buffer;

        GraphicsEnum graphics = Amara::GraphicsEnum::None;
        
        #ifdef AMARA_OPENGL
        SDL_GLContext glContext = NULL;
        #endif
        SDL_GPUDevice* gpuDevice = nullptr;
        SDL_Renderer* renderer = nullptr;

        GPUHandler* gpuHandler = nullptr;
        RenderBatch* renderBatch = nullptr;

        bool glFunctionsLoaded = false;

        #if AMARA_OPENGL
        ShaderProgram* currentShaderProgram = nullptr;
        ShaderProgram* defaultShaderProgram = nullptr;
        #endif

        SDL_Window* current_window = nullptr;
        Rectangle window_dim;
        SDL_DisplayID displayID = 0;

        GameManager* game = nullptr;
        World* world = nullptr;
        Scene* scene = nullptr;

        std::vector<World*>* world_list = nullptr;
        double worldLifetime = 0;

        void breakWorld();

        NodeFactory* factory = nullptr;
        ScriptFactory* scripts = nullptr;
        SystemManager* system = nullptr;
        InputManager* inputManager = nullptr;
        AssetManager* assets = nullptr;
        AnimationFactory* animations = nullptr;
        ShaderManager* shaders = nullptr;
        AudioMaster* audio = nullptr;
        MessageQueue* messages = nullptr;
        GarbageCollector* garbageCollector = nullptr;

        std::vector<SDL_Keycode> key_pressed_buffer;
        std::vector<SDL_Keycode> key_released_buffer;

        bool lua_exception_thrown = false;

        void queue_garbage(Amara::Node*, double);
        void queue_garbage(Amara::Node* node) {
            queue_garbage(node, 1);
        }
        void queue_asset_garbage(Amara::Asset* asset, double expiration);
        void queue_asset_garbage(Amara::Asset* asset) {
            queue_asset_garbage(asset, 1);
        }
        #ifdef AMARA_OPENGL
        void queue_texture_garbage(GLuint textureID);
        #endif

        KeyboardManager* keyboard = nullptr;
        GamepadManager* gamepads = nullptr;
        ControlManager* controls = nullptr;

        Pointer* globalPointer = nullptr;
    };
}