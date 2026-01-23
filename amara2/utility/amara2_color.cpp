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
        Color(sol::object config) {
            *this = config;
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

        Amara::Color& operator= (const SDL_Color& _color) {
            r = _color.r;
            g = _color.g;
            b = _color.b;
            a = _color.a;
            return *this;
        }
        Amara::Color& operator= (nlohmann::json config) {
            configure(config);
            return *this; 
        }
        Amara::Color& operator= (sol::object config);

        Amara::Color operator* (const Amara::Color& other) const {
            return Amara::Color(
                (r * other.r) / 255,
                (g * other.g) / 255,
                (b * other.b) / 255,
                (a * other.a) / 255
            );
        }
        Amara::Color operator* (float other) const {
            return Amara::Color(
                r * other,
                g * other,
                b * other,
                a * other
            );
        }

        Amara::Color operator+ (const Amara::Color& other) const {
            return Amara::Color(
                r + other.r,
                g + other.g,
                b + other.b,
                a + other.a
            );
        }
        Amara::Color operator- (const Amara::Color& other) const {
            return Amara::Color(
                r - other.r,
                g - other.g,
                b - other.b,
                a - other.a
            );
        }
        
        explicit operator std::string() const {
            return "Color(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(a) + ")";
        }
        friend std::ostream& operator<<(std::ostream& os, const Color& color) {
            return os << static_cast<std::string>(color);
        }

        std::string hex() {
            std::ostringstream oss;
            oss << "#";
            oss << std::hex << std::uppercase << std::setfill('0');
            oss << std::setw(2) << static_cast<int>(r)
                << std::setw(2) << static_cast<int>(g)
                << std::setw(2) << static_cast<int>(b)
                << std::setw(2) << static_cast<int>(a);
            return oss.str();
        }

        static bool isColor(std::string key) {
            if (key[0] == '#') {
                if (key.length() == 7 || key.length() == 9) return true;
                return false;
            }
            if (String::equal(key, "white")) return true;
            if (String::equal(key, "black")) return true;
            if (String::equal(key, "red")) return true;
            if (String::equal(key, "green")) return true;
            if (String::equal(key, "blue")) return true;
            if (String::equal(key, "yellow")) return true;
            if (String::equal(key, "magenta")) return true;
            if (String::equal(key, "cyan")) return true;
            if (String::equal(key, "orange")) return true;
            if (String::equal(key, "purple")) return true;
            if (String::equal(key, "brown")) return true;
            if (String::equal(key, "pink")) return true;
            if (String::equal(key, "gray")) return true;
            if (String::equal(key, "grey")) return true;
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
                std::string color_str = config.get<std::string>();

                if (color_str[0] == '#') {
                    color_str = color_str.substr(1); // Remove the leading '#'
                    
                    // Color in hex format #FFFFFF
                    if (color_str.length() == 6) {
                        r = std::stoi(color_str.substr(0, 2), nullptr, 16);
                        g = std::stoi(color_str.substr(2, 2), nullptr, 16);
                        b = std::stoi(color_str.substr(4, 2), nullptr, 16);
                        a = 255;
                    }
                    else if (color_str.length() == 8) {
                        r = std::stoi(color_str.substr(0, 2), nullptr, 16);
                        g = std::stoi(color_str.substr(2, 2), nullptr, 16);
                        b = std::stoi(color_str.substr(4, 2), nullptr, 16);
                        a = std::stoi(color_str.substr(6, 2), nullptr, 16);
                    }
                    else {
                        fatal_error("Error: Invalid color format: " + color_str);
                    }
                }
                else if (String::equal(color_str, "white")) {
                    *this = Color::White;
                }
                else if (String::equal(color_str, "black")) {
                    *this = Color::Black;
                }
                else if (String::equal(color_str, "red")) {
                    *this = Color::Red;
                }
                else if (String::equal(color_str, "green")) {
                    *this = Color::Green;
                }
                else if (String::equal(color_str, "blue")) {
                    *this = Color::Blue;
                }
                else if (String::equal(color_str, "yellow")) {
                    *this = Color::Yellow;
                }
                else if (String::equal(color_str, "magenta")) {
                    *this = Color::Magenta;
                }
                else if (String::equal(color_str, "cyan")) {
                    *this = Color::Cyan;
                }
                else if (String::equal(color_str, "orange")) {
                    *this = Color::Orange;
                }
                else if (String::equal(color_str, "purple")) {
                    *this = Color::Purple;
                }
                else if (String::equal(color_str, "brown")) {
                    *this = Color::Brown;
                }
                else if (String::equal(color_str, "pink")) {
                    *this = Color::Pink;
                }
                else if (String::equal(color_str, "gray")) {
                    *this = Color::Gray;
                }
                else if (String::equal(color_str, "grey")) {
                    *this = Color::Gray;
                }
                else if (String::equal(color_str, "transparent")) {
                    *this = Color::Transparent;
                }
                else {
                    fatal_error("Error: Invalid color: " + color_str);
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
                if (!json_has_any(config, "r", "g", "b", "a")) {
                    fatal_error("Error: Invalid Color assignment.");
                }
            }
            else {
                fatal_error("Error: Invalid Color assignment.");
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
        static Color Orange;
        static Color Purple;
        static Color Brown;
        static Color Pink;
        static Color Gray;
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
    Color Color::Orange = {255, 165, 0, 255};
    Color Color::Purple = {128, 0, 128, 255};
    Color Color::Brown = {165, 42, 42, 255};
    Color Color::Pink = {255, 192, 203, 255};
    Color Color::Gray = {128, 128, 128, 255};
    Color Color::Transparent = {0, 0, 0, 0};

    void Color::bind_lua(sol::state& lua) {
        sol::usertype<Color> color_type = lua.new_usertype<Color>("Color",
            sol::constructors<Color(Uint8, Uint8, Uint8, Uint8), Color(Uint8, Uint8, Uint8), Color(sol::object)>(),
            "r", &Color::r,
            "g", &Color::g,
            "b", &Color::b,
            "a", &Color::a,
            "hex", &Color::hex,
            sol::meta_function::multiplication, sol::overload(
                sol::resolve<Amara::Color(const Amara::Color&) const>(&Amara::Color::operator*),
                sol::resolve<Amara::Color(float) const>(&Amara::Color::operator*)
            ),
            sol::meta_function::addition, &Amara::Color::operator+,
            sol::meta_function::subtraction, &Amara::Color::operator-,
            "normalize", [&lua](const Color& self) {
                sol::table normalized = lua.create_table();
                normalized[1] = self.r / 255.0f;
                normalized[2] = self.g / 255.0f;
                normalized[3] = self.b / 255.0f;
                normalized[4] = self.a / 255.0f;
                return normalized;
            },
            sol::meta_function::addition, &Amara::Color::operator+,
            sol::meta_function::subtraction, &Amara::Color::operator-,
            sol::meta_function::multiplication, [](const Color& self, sol::object val) {
                if (val.get_type() == sol::type::number) {
                    float scalar = val.as<float>();
                    return self * scalar;
                }
                else if (val.is<Color>() || val.is<sol::table>() || val.is<std::string>()) {
                    const Color& other = val;
                    return self * other;
                }
                else {
                    fatal_error("Error: Invalid type for Color multiplication");
                }
            }
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
            "Orange", Color::Orange,
            "Purple", Color::Purple,
            "Brown", Color::Brown,
            "Pink", Color::Pink,
            "Gray", Color::Gray,
            "Grey", Color::Gray,
            "Transparent", Color::Transparent
        );
    }
}