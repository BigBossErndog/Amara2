namespace Amara {
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
        Color& operator= (sol::object config);

        explicit operator std::string() const {
            return "Color(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(a) + ")";
        }
        friend std::ostream& operator<<(std::ostream& os, const Color& color) {
            return os << static_cast<std::string>(color);
        }

        static bool isColor(std::string key) {
            if (String::equal(key, "white")) return true;
            if (String::equal(key, "black")) return true;
            if (String::equal(key, "red")) return true;
            if (String::equal(key, "green")) return true;
            if (String::equal(key, "blue")) return true;
            if (String::equal(key, "yellow")) return true;
            if (String::equal(key, "magenta")) return true;
            if (String::equal(key, "cyan")) return true;
            if (String::equal(key, "transparent")) return true;
            return false;
        }

        Amara::Color& configure(nlohmann::json config) {
            if (config.is_array()) {
                if (config.size() >= 3) {
                    r = config[0];
                    g = config[1];
                    b = config[2];
                    a = 255;
                }
                if (config.size() >= 4) {
                    a = config[3];
                }
            }
            else if (config.is_string()) {
                if (String::equal(config, "white")) {
                    r = 255; g = 255; b = 255; a = 255;
                }
                else if (String::equal(config, "black")) {
                    r = 0; g = 0; b = 0; a = 255;
                }
                else if (String::equal(config, "red")) {
                    r = 255; g = 0; b = 0; a = 255;
                }
                else if (String::equal(config, "green")) {
                    r = 0; g = 255; b = 0; a = 255;
                }
                else if (String::equal(config, "blue")) {
                    r = 0; g = 0; b = 255; a = 255;
                } 
                else if (String::equal(config, "yellow")) {
                    r = 255; g = 255; b = 0; a = 255;
                }
                else if (String::equal(config, "magenta")) {
                    r = 255; g = 0; b = 255; a = 255;
                }
                else if (String::equal(config, "cyan")) {
                    r = 0; g = 255; b = 255; a = 255;
                }
                else if (String::equal(config, "transparent")) {
                    r = 0; g = 0; b = 0; a = 0;
                }
                else {
                    // Color in hex format #FFFFFF
                    std::string hex_color = config.get<std::string>();
                    if (hex_color[0] == '#') {
                        hex_color = hex_color.substr(1);

                        if (hex_color.length() == 6) {
                            r = std::stoi(hex_color.substr(0, 2), nullptr, 16);
                            g = std::stoi(hex_color.substr(2, 2), nullptr, 16);
                            b = std::stoi(hex_color.substr(4, 2), nullptr, 16);
                            a = 255;
                        } else if (hex_color.length() == 8) {
                            r = std::stoi(hex_color.substr(0, 2), nullptr, 16);
                            g = std::stoi(hex_color.substr(2, 2), nullptr, 16);
                            b = std::stoi(hex_color.substr(4, 2), nullptr, 16);
                            a = std::stoi(hex_color.substr(6, 2), nullptr, 16);
                        }
                    }
                    else {
                        r = 0; g = 0; b = 0; a = 255;
                    }
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "r")) r = config["r"];
                else r = 0;
                if (json_has(config, "g")) g = config["g"];
                else g = 0;
                if (json_has(config, "b")) b = config["b"];
                else b = 0;
                if (json_has(config, "a")) a = config["a"];
                else a = 255;
            }
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

        static void bind_lua(sol::state& lua);
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

    void Color::bind_lua(sol::state& lua) {
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