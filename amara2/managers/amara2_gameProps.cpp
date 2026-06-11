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
        float pitch = 1.0f;
        
        void reset() {
            AudioData new_data;
            new_data.device = device;

            volume = 1;
            panning = 0;
            
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
        
        Vector2 input_scale = { 1, 1 };
        
        bool insideTextureContainer = false;
        bool texturePropsLock = false;
        bool inputDisabled = false;
        bool insideCamera = false;
        
        void reset(bool keep_texture_state) {
            PassOnProps new_props;
            new_props.window_zoom = window_zoom;

            if (keep_texture_state) {
                new_props.insideTextureContainer = insideTextureContainer;
                new_props.texturePropsLock = texturePropsLock;
                new_props.input_scale = input_scale;
            }
            
            *this = new_props;
        }
        void reset() {
            reset(false);
        }
    };
    
    class GameProps {
    public:
        sol::state lua;
        int lua_stack_size = 1000000000;

        #ifdef AMARA_TEST_PATH
        std::string context_path = AMARA_TEST_PATH;
        #else
        std::string context_path = "./";
        #endif
        std::string lua_script_path = "./lua_scripts";
        std::string assets_path = "./assets";

        #ifdef AMARA_DEF_ORG
        std::string define_org = AMARA_ORG;
        #else
        std::string define_org = "";
        #endif

        #ifdef AMARA_DEF_APP
        std::string define_app = AMARA_DEF_APP;
        #else
        std::string define_app = "";
        #endif

        bool integrate_new_windows = false;
        bool transparent_window = false;

        double deltaTime = 1;
        
        Amara::World* render_origin = nullptr;
        Rectangle master_viewport;
        Rectangle display;

        PassOnProps passOn;
        AudioData audioData;

        GraphicsEnum graphics = Amara::GraphicsEnum::None;
        
        #ifdef AMARA_OPENGL
        SDL_GLContext glContext = NULL;
        #endif
        SDL_GPUDevice* gpuDevice = nullptr;
        SDL_Renderer* renderer = nullptr;

        GPUHandler* gpuHandler = nullptr;
        RenderBatch* renderBatch = nullptr;

        bool glFunctionsLoaded = false;

        #ifdef AMARA_OPENGL
        ShaderProgram* currentShaderProgram = nullptr;
        ShaderProgram* defaultShaderProgram = nullptr;
        #endif

        SDL_Window* current_window = nullptr;
        Rectangle window_dim;
        SDL_DisplayID displayID = 0;
        SDL_DisplayID targetDisplayID = 0;

        TextInputEnum text_input_type = TextInputEnum::None;
        std::string text_input;
        int text_input_count = 0;
        
        bool recording_text_input() {
            return text_input_count > 0;
        }

        GameManager* game = nullptr;
        World* world = nullptr;
        Scene* scene = nullptr;

        std::vector<World*>* world_list = nullptr;
        double worldLifetime = 0;

        bool lua_exception_thrown = false;
        int error_code = 0;

        void breakWorld();
        void breakWorld(int);
        
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

        Amara::InputMode controlMode = Amara::InputMode::None;

        Pointer* globalPointer = nullptr;

        CursorEnum current_cursor = CursorEnum::Default;
        SDL_Cursor* cursor_default = nullptr;
        SDL_Cursor* cursor_pointer = nullptr;
        SDL_Cursor* cursor_text = nullptr;
        SDL_Cursor* cursor_wait = nullptr;
        SDL_Cursor* cursor_progress = nullptr;
        SDL_Cursor* cursor_notAllowed = nullptr;
    };
}