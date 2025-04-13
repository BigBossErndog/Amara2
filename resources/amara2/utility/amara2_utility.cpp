namespace Amara {
    template<typename... Args>
    void debug_log(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        std::cout << ss.str().c_str() << std::endl;
    }

    template <class T> bool vector_contains(std::vector<T> list, T f) {
        for (T obj: list) 
            if (obj == f) return true;
        return false;
    }
    
    template <class T> void vector_append(std::vector<T>& list1, std::vector<T> list2) {
        for (T element: list2) list1.push_back(element);
    }

    bool json_has(const nlohmann::json& data, std::string key) {
        return (data.find(key) != data.end()) ? true : false;
    }

    bool json_is(const nlohmann::json& data, std::string key) {
        return json_has(data, key) && data[key].is_boolean() && data[key];
    }

    bool json_erase(nlohmann::json& data, std::string key) {
        if (json_has(data, key)) {
            data.erase(key);
            return true;
        }
        return false;
    }

    template<typename... Args>
    std::string string_concat(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        return ss.str();
    }

    template<typename... Args>
    std::string string_sep_concat(const std::string& separator, Args ... args) {
        std::ostringstream ss;
        bool first = true;
        ((ss << (first ? "" : separator) << args, first = false), ...);
        return ss.str();
    }

    bool string_endsWith(std::string str, std::string suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
    }

    bool string_startsWith(std::string& str, std::string& prefix) {
        if (prefix.size() > str.size()) {
            return false;
        }
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

    bool string_equal(std::string str1, std::string str2) {
        return str1.compare(str2) == 0;
    }
    
    std::string float_string(float n) {
        if (floor(n) == n) return (std::to_string((int)n));
        nlohmann::json json(n);
        return json.dump();
    }

    float fixed_range(float num, float min, float max) {
        if (num < min) return min;
        if (num > max) return max;
        return num;
    }
    
    float abs_mod(float num, float den) {
        while (num < 0) num += den;
        float result = fmod(num, den);
        return result;
    }

    bool is_node(sol::object);

    std::string graphics_to_string(GraphicsEnum g) {
        switch (g) {
            case GraphicsEnum::Render2D: return "Render2D";
            case GraphicsEnum::OpenGL: return "OpenGL";
            case GraphicsEnum::Vulkan: return "Vulkan";
            case GraphicsEnum::DirectX: return "DirectX";
            case GraphicsEnum::DirectX_Legacy: return "DirectX_Legacy";
            case GraphicsEnum::VulkanMetalDirectX: return "GPURenderer";
            default: return "None";
        }
    }

    #if defined(_WIN32)
        std::vector<GraphicsEnum> Amara_Default_Graphics_Priority = {
            GraphicsEnum::Render2D,
            #ifdef AMARA_OPENGL
            GraphicsEnum::OpenGL,
            #endif
            GraphicsEnum::VulkanMetalDirectX
            
        };
    #else
        std::vector<GraphicsEnum> Amara_Default_Graphics_Priority = {
            GraphicsEnum::Render2D,
            #ifdef AMARA_OPENGL
            GraphicsEnum::OpenGL
            #endif
            GraphicsEnum::VulkanMetalDirectX,
        };
    #endif

    struct Color: public SDL_Color {
        Color() = default;
        Color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) {
            r = _r;
            g = _g;
            b = _b;
            a = _a;
        }
        Color(Uint8 _r, Uint8 _g, Uint8 _b): Color(_r, _g, _b, 255) {}
        Color(SDL_Color _color) {
            r = _color.r;
            g = _color.g;
            b = _color.b;
            a = _color.a;
        }
        Color(nlohmann::json config) {
            configure(config);
        }

        void toFloats(float* _r, float* _g, float* _b) {
            (*_r) = r;
            (*_g) = g;
            (*_b) = b;
        }
        void toFloats(float* _r, float* _g, float* _b, float* _a) {
            (*_a) = a;
            toFloats(_r, _g, _b);
        }

        Color& operator= (const SDL_Color& _color) {
            r = _color.r;
            g = _color.g;
            b = _color.b;
            a = _color.a;
            return *this;
        }
        Color& operator= (nlohmann::json config) {
            configure(config);
            return *this; 
        }

        Amara::Color& configure(nlohmann::json config) {
            if (json_has(config, "r")) r = config["r"];
            else r = 0;
            if (json_has(config, "g")) g = config["g"];
            else g = 0;
            if (json_has(config, "b")) b = config["b"];
            else b = 0;
            if (json_has(config, "a")) a = config["a"];
            else a = 255;
            return *this;
        }

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                { "r", r },
                { "g", g },
                { "b", b },
                { "a", a }
            });
        }

        static Color White;
        static Color Black;
        static Color Red;
        static Color Green;
        static Color Blue;
        static Color Yellow;
        static Color Magenta;
        static Color Cyan;
        static Color Transparent;

        static void bindLua(sol::state& lua);
    };

    Color Color::White = {255, 255, 255, 255};
    Color Color::Black = {0, 0, 0, 255};
    Color Color::Red = {255, 0, 0, 255};
    Color Color::Green = {0, 255, 0, 255};
    Color Color::Blue = {0, 0, 255, 255};
    Color Color::Yellow = {255, 255, 0, 255};
    Color Color::Magenta = {255, 0, 255, 255};
    Color Color::Cyan = {0, 255, 255, 255};
    Color Color::Transparent = {0, 0, 0, 0};

    void Color::bindLua(sol::state& lua) {
        sol::usertype<Color> color_type = lua.new_usertype<Color>("Color",
            sol::constructors<Color(Uint8, Uint8, Uint8, Uint8), Color(Uint8, Uint8, Uint8)>(),
            "r", &Color::r,
            "g", &Color::g,
            "b", &Color::b,
            "a", &Color::a
        );

        lua.new_enum("Colors",
            "White", Color::White,
            "Black", Color::Black,
            "Red", Color::Red,
            "Green", Color::Green,
            "Blue", Color::Blue,
            "Yellow", Color::Yellow,
            "Magenta", Color::Magenta,
            "Cyan", Color::Cyan,
            "Transparent", Color::Transparent
        );
    }
}