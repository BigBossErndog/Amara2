namespace Amara {    
    struct Vector2 {
        Vector2() = default;
        Vector2(float _g) : x(_g), y(_g) {}
        Vector2(float x_, float y_) : x(x_), y(y_) {}
        Vector2(const SDL_FPoint& p): Vector2(p.x, p.y) {}
        Vector2(nlohmann::json config) {
            *this = config;
        }
        Vector2(sol::object obj) {
            *this = obj;
        }

        float x = 0;
        float y = 0;

        Vector2 operator* (float scalar) const {
            return Vector2(x * scalar, y * scalar);
        }
        Vector2& operator*= (float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }
        Vector2 operator* (const Vector2& other) const {
            return Vector2(x * other.x, y * other.y);
        }
        Vector2& operator*= (const Vector2& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        Vector2 operator/ (float scalar) const {
            return Vector2(x / scalar, y / scalar);
        }
        Vector2& operator/= (float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
        Vector2 operator/ (const Vector2& other) const {
            return Vector2(x / other.x, y / other.y);
        }
        Vector2& operator/= (const Vector2& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        Vector2 operator+ (const Vector2& other) const {
            return Vector2(x + other.x, y + other.y);
        }
        Vector2& operator+= (const Vector2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2 operator- (const Vector2& other) const {
            return Vector2(x - other.x, y - other.y);
        }
        Vector2& operator-= (const Vector2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        
        bool operator== (const Vector2& other) const {
            return x == other.x && y == other.y;
        }

        explicit operator std::string() const {
            return "{ x: " + String::float_to_string(x) + ", y: " + String::float_to_string(y) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
            return os << static_cast<std::string>(v);
        }

        // Cross product (for 2D vector)
        float cross(const Vector2& v) const {
            return x * v.y - y * v.x;
        }

        // Dot product
        float dot(const Vector2& v) const {
            return x * v.x + y * v.y;
        }

        // Length (magnitude) of the vector
        float length() const {
            return std::sqrt(x * x + y * y);
        }

        // Alias for length (magnitude)
        float magnitude() const {
            return length();
        }

        // Normalize the vector
        Vector2 normalize() {
            float length = std::sqrt(x * x + y * y);
            if (length > 0) {
                x /= length;
                y /= length;
            }
            return *this;
        }

        Vector2 normalized() const {
            Vector2 result = *this;
            result.normalize();
            return result;
        }

        // Reflect the vector off a normal
        Vector2 reflect(const Vector2& normal) const {
            return *this - normal * 2.0f * this->dot(normal);
        }
    
        Vector2 project(const Vector2& other) const {
            if (other.length() == 0) {
                return Vector2::Origin;
            }
            float dot_product = dot(other);
            float other_magnitude_squared = other.length() * other.length();
            return other * (dot_product / other_magnitude_squared);
        }
    
        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y}
            });
        }

        Vector2& operator= (nlohmann::json config);
        Vector2& operator= (sol::object obj);

        Vector2 round() {
            return Vector2(std::round(x), std::round(y));
        }
        Vector2 floor() {
            return Vector2(std::floor(x), std::floor(y));
        }
        Vector2 ceil() {
            return Vector2(std::ceil(x), std::ceil(y));
        }
        
        static Vector2 Origin;
        static Vector2 Zero;
        static Vector2 Left;
        static Vector2 Right;
        static Vector2 Top;
        static Vector2 Bottom;
        static Vector2 TopLeft;
        static Vector2 TopRight;
        static Vector2 BottomLeft;
        static Vector2 BottomRight;
        static Vector2 Center;
    };
    
    Vector2 Vector2::Origin = Vector2(0, 0);
    Vector2 Vector2::Zero = Vector2(0, 0);
    Vector2 Vector2::Left = Vector2(0, 0.5);
    Vector2 Vector2::Right = Vector2(1, 0.5);
    Vector2 Vector2::Top = Vector2(0.5, 0);
    Vector2 Vector2::Bottom = Vector2(0.5, 1);
    Vector2 Vector2::TopLeft = Vector2(0, 0);
    Vector2 Vector2::TopRight = Vector2(1, 0);
    Vector2 Vector2::BottomLeft = Vector2(0, 1);
    Vector2 Vector2::BottomRight = Vector2(1, 1);
    Vector2 Vector2::Center = Vector2(0.5, 0.5);

    struct Vector3: public Vector2 {
        Vector3() = default;
        Vector3(float _g) : Vector2(_g, _g) {}
        Vector3(float _x, float _y, float _z) : Vector2(_x, _y), z(_z) {}
        Vector3(const Vector2& v2) : Vector2(v2) {}
        Vector3(const Vector2& v2, float _z): Vector2(v2.x, v2.y), z(_z) {}
        Vector3(nlohmann::json config) {
            *this = config;
        }
        Vector3(sol::object obj) {
            *this = obj;
        }

        public: float z = 0;

        Vector3 operator+ (const Vector3& other) const {
            return Vector3(x + other.x, y + other.y, z + other.z);
        }
        Vector3 operator+ (const Vector2& other) const {
            return Vector3(x + other.x, y + other.y, z);
        }
        Vector3& operator+=(const Vector3& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }
        Vector3& operator+=(const Vector2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector3 operator- (const Vector3& other) const {
            return Vector3(x + other.x, y + other.y, z);
        }
        Vector3 operator- (const Vector2& other) const {
            return Vector3(x - other.x, y - other.y, z);
        }
        Vector3& operator-=(const Vector3& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }
        Vector3& operator-=(const Vector2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vector3 operator* (float scalar) const {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }
        Vector3 operator* (const Vector3& other) const {
            return Vector3(x * other.x, y * other.y, z * other.z);
        }
        Vector3 operator* (const Vector2& other) const {
            return Vector3(x * other.x, y * other.y, z);
        }
        Vector3 operator/ (float scalar) const {
            return Vector3(x / scalar, y / scalar, z / scalar);
        }
        Vector3 operator/ (const Vector3& other) const {
            return Vector3(x / other.x, y / other.y, z / other.z);
        }
        Vector3 operator/ (const Vector2& other) const {
            return Vector3(x / other.x, y / other.y, z);
        }
        
        Vector3& operator*= (float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }
        Vector3& operator*= (const Vector3& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }
        Vector3& operator*= (const Vector2& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }
        Vector3& operator/= (float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }
        Vector3& operator/= (const Vector3& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }
        Vector3& operator/= (const Vector2& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y},
                {"z", z}
            });
        }

        Vector3& operator= (nlohmann::json config) {
            if (config.is_array()) {
                if (config.size() == 3) {
                    x = config[0];
                    y = config[1];
                    z = config[2];
                }
                else if (config.size() == 2) {
                    x = config[0];
                    y = config[1];
                }
                else {
                    fatal_error("Error: Invalid Vector3 assignment.");
                }
            }
            else if (config.is_number()) {
                x = y = z = config.get<float>();
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];
                if (json_has(config, "z")) z = config["z"];
                if (!json_has_any(config, "x", "y", "z")) {
                    fatal_error("Error: Invalid Vector3 assignment.");
                }
            }
            else {
                fatal_error("Error: Invalid Vector3 assignment.");
            }
            
            return *this;
        }
        Vector3& operator= (sol::object obj);

        Vector3& operator= (const Vector2& v2) {
            x = v2.x;
            y = v2.y;
            return *this;
        }

        bool operator==(const Vector3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        explicit operator std::string() const {
            return "{ x: " + String::float_to_string(x) + ", y: " + String::float_to_string(y) + ", z: " + String::float_to_string(z) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
            return os << static_cast<std::string>(v);
        }
    };
    
    struct Vector4: public Vector3 {
        Vector4() = default;
        Vector4(float _x, float _y, float _z, float _w) : Vector3(_x, _y, _z), w(_w) {}
        Vector4(nlohmann::json config) {
            *this = config;
        }
        Vector4(sol::object obj) {
            *this = obj;
        }

        public: float w = 0;

        Vector4 operator+ (const Vector4& other) const {
            return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
        }
        Vector4& operator+=(const Vector4& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        Vector4 operator- (const Vector4& other) const {
            return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
        }
        Vector4& operator-=(const Vector4& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        Vector4 operator* (float scalar) const {
            return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
        }
        Vector4 operator* (const Vector4& other) const {
            return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
        }
        
        Vector4 operator/ (float scalar) const {
            return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
        }
        Vector4 operator/ (const Vector4& other) const {
            return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
        }
        
        Vector4& operator*= (float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }
        Vector4& operator*= (const Vector4& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }
        Vector4& operator/= (float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
            return *this;
        }
        Vector4& operator/= (const Vector4& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            w /= other.w;
            return *this;
        }

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y},
                {"z", z},
                {"w", w}
            });
        }

        Vector4& operator= (nlohmann::json config) {
            if (config.is_array()) {
                if (config.size() == 4) {
                    x = config[0];
                    y = config[1];
                    z = config[2];
                    w = config[3];
                }
                else if (config.size() == 3) {
                    x = config[0];
                    y = config[1];
                    z = config[2];
                }
                else if (config.size() == 2) {
                    x = config[0];
                    y = config[1];
                }
                else {
                    fatal_error("Error: Invalid Vector4 assignment.");
                }
            }
            else if (config.is_number()) {
                x = y = z = w = config.get<float>();
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];
                if (json_has(config, "z")) z = config["z"];
                if (json_has(config, "w")) w = config["w"];
                if (!json_has_any(config, "x", "y", "z", "w")) {
                    fatal_error("Error: Invalid Vector4 assignment.");
                }
            }
            else {
                fatal_error("Error: Invalid Vector4 assignment.");
            }
            return *this;
        }
        Vector4& operator= (sol::object obj);

        Vector4& operator= (const Vector3& v3) {
            x = v3.x;
            y = v3.y;
            z = v3.z;
            return *this;
        }
        Vector4& operator= (const Vector2& v2) {
            x = v2.x;
            y = v2.y;
            return *this;
        }

        bool operator==(const Vector4& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        explicit operator std::string() const {
            return "{ x: " + String::float_to_string(x) + ", y: " + String::float_to_string(y) + ", z: " + String::float_to_string(z) + ", w: " + String::float_to_string(w) + " }";
        }

        friend std::ostream& operator<<(std::ostream& os, const Vector4& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Matrix4x4 {
        Matrix4x4() = default;
        Matrix4x4(
            float _m11, float _m12, float _m13, float _m14,
            float _m21, float _m22, float _m23, float _m24,
            float _m31, float _m32, float _m33, float _m34,
            float _m41, float _m42, float _m43, float _m44
        ) {
            m11 = _m11; m12 = _m12; m13 = _m13; m14 = _m14;
            m21 = _m21; m22 = _m22; m23 = _m23; m24 = _m24;
            m31 = _m31; m32 = _m32; m33 = _m33; m34 = _m34;
            m41 = _m41; m42 = _m42; m43 = _m43; m44 = _m44;
        }

        float m11, m12, m13, m14;
        float m21, m22, m23, m24;
        float m31, m32, m33, m34;
        float m41, m42, m43, m44;
    };

    void bind_lua_Vectors(sol::state& lua) {
        lua.new_usertype<Vector2>("Vector2",
            sol::constructors<Vector2(), Vector2(float, float)>(),
            "x", &Vector2::x,
            "y", &Vector2::y,
            sol::meta_function::addition, [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v + other;
            },
            "move", [](Vector2& v, sol::object val) {
                Vector2 other = val;
                v.x += other.x;
                v.y += other.y;
            },
            sol::meta_function::subtraction, [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v - other;
            },
            sol::meta_function::multiplication, [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v * other;
            },
            sol::meta_function::division, [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v / other;
            },
            "cross", [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v.cross(other);
            },
            "dot", [](Vector2& v, sol::object val) {
                Vector2 other = val;
                return v.dot(other);
            },
            "length", sol::property([](const Vector2& v) {
                return v.length();
            }),
            "magnitude", sol::property([](const Vector2& v) {
                return v.magnitude();
            }),
            "normalized", sol::property([](const Vector2& v) {
                return v.normalized();
            }),
            "string", sol::property([](const Vector2& v) {
                return std::string(v);
            }),
            sol::meta_function::equal_to, [](const Vector2& v, sol::object val) {
                Vector2 other = val;
                return v == other;
            },
            sol::meta_function::to_string, [](const Vector2& v) {
                return std::string(v);
            }
        );
        lua.new_usertype<Vector3>("Vector3",
            sol::constructors<Vector3(), Vector3(float, float, float), Vector3(const Vector2&, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "z", &Vector3::z,
            sol::meta_function::addition, [](const Vector3& v, sol::object val) {
                Vector3 other = val;
                return v + other;
            },
            "move", [](Vector3& v, sol::object val) {
                Vector3 other = val;
                v.x += other.x;
                v.y += other.y;
                v.z += other.z;
                return &v;
            },
            sol::meta_function::subtraction, [](const Vector3& v, sol::object val) {
                Vector3 other = val;
                return v - other;
            },
            sol::meta_function::multiplication, [](const Vector3& v, sol::object val) {
                Vector3 other = val;
                return v * other;
            },
            sol::meta_function::division, [](Vector3& v, sol::object val) {
                Vector3 other = val;
                return v / other;
            },
            sol::meta_function::equal_to, [](const Vector3& v, sol::object val) {
                Vector3 other = val;
                return v == other;
            },
            sol::meta_function::to_string, [](const Vector3& v) {
                return std::string(v);
            },
            "string", sol::property([](const Vector3& v) {
                return std::string(v);
            })
        );
        
        lua.new_usertype<Vector4>("Vector4",
            sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
            "x", &Vector4::x,
            "y", &Vector4::y,
            "z", &Vector4::z,
            "w", &Vector4::w,
            sol::meta_function::addition, [](Vector4& v, sol::object val) {
                Vector4 other = val;
                return v + other;
            },
            sol::meta_function::subtraction, [](Vector4& v, sol::object val) {
                Vector4 other = val;
                return v - other;
            },
            sol::meta_function::multiplication, [](Vector4& v, sol::object val) {
                Vector4 scalar = val;
                return v * scalar;
            },
            sol::meta_function::division, [](Vector4& v, sol::object val) {
                Vector4 scalar = val;
                return v / scalar;
            },
            sol::meta_function::equal_to, [](const Vector4& v, sol::object val) {
                Vector4 other = val;
                return v == other;
            },
            sol::meta_function::to_string, [](const Vector4& v) {
                return std::string(v);
            }
        );
    }
}