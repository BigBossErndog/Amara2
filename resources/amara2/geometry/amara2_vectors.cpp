namespace Amara {    
    struct Vector2 {
        Vector2() = default;
        Vector2(float x_, float y_) : x(x_), y(y_) {}
        Vector2(const SDL_FPoint& p): Vector2(p.x, p.y) {}

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
            return "{ x: " + float_string(x) + ", y: " + float_string(y) + " }";
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

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y}
            });
        }

        Vector2& operator= (nlohmann::json config);
        Vector2& operator= (sol::object obj);

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

    Vector2 Vector2::Left = Vector2(0, 0);
    Vector2 Vector2::Right = Vector2(1, 0);
    Vector2 Vector2::Top = Vector2(0, 1);
    Vector2 Vector2::Bottom = Vector2(0, -1);
    Vector2 Vector2::TopLeft = Vector2(0, 0);
    Vector2 Vector2::TopRight = Vector2(1, 0);
    Vector2 Vector2::BottomLeft = Vector2(0, 1);
    Vector2 Vector2::BottomRight = Vector2(1, 1);
    Vector2 Vector2::Center = Vector2(0.5, 0.5);

    struct Vector3: public Vector2 {
        Vector3() = default;
        Vector3(float _x, float _y, float _z) : Vector2(_x, _y), z(_z) {}
        Vector3(const Vector2& v2, float _z): Vector2(v2.x, v2.y), z(_z) {}

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

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y},
                {"z", z}
            });
        }

        Vector3& operator= (nlohmann::json config) {
            if (json_has(config, "x")) x = config["x"];
            if (json_has(config, "y")) y = config["y"];
            if (json_has(config, "z")) z = config["z"];
            return *this;
        }
        Vector3& operator= (sol::object obj);

        bool operator==(const Vector3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        explicit operator std::string() const {
            return "{ x: " + float_string(x) + ", y: " + float_string(y) + ", z: " + float_string(z) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Matrix4x4 {
        float m11, m12, m13, m14;
        float m21, m22, m23, m24;
        float m31, m32, m33, m34;
        float m41, m42, m43, m44;
    };

    void bindLua_Vectors(sol::state& lua) {
        lua.new_usertype<Vector2>("Vector2",
            sol::constructors<Vector2(), Vector2(float, float)>(),
            "x", &Vector2::x,
            "y", &Vector2::y,
            sol::meta_function::addition, &Vector2::operator+,
            "add", sol::overload(
                &Vector2::operator+=,
                [](Vector2& v, float _x, float _y) -> Vector2* {
                    v.x += _x;
                    v.y += _y;
                    return &v;
                }
            ),
            "move", sol::overload(
                &Vector2::operator+=,
                [](Vector2& v, float _x, float _y) -> Vector2* {
                    v.x += _x;
                    v.y += _y;
                    return &v;
                }
            ),
            sol::meta_function::subtraction, &Vector2::operator-,
            "subtract", sol::overload(
                &Vector2::operator-=,
                [](Vector2& v, float _x, float _y) -> Vector2* {
                    v.x -= _x;
                    v.y -= _y;
                    return &v;
                }
            ),
            sol::meta_function::multiplication, sol::overload(
                sol::resolve<Vector2(float) const>(&Vector2::operator*),
                sol::resolve<Vector2(const Vector2&) const>(&Vector2::operator*)
            ),
            "multiply", sol::overload(
                sol::resolve<Vector2&(float)>(&Vector2::operator*=),
                sol::resolve<Vector2&(const Vector2&)>(&Vector2::operator*=),
                [](Vector2& v, float _x, float _y) -> Vector2* {
                    v.x *= _x;
                    v.y *= _y;
                    return &v;
                }
            ),
            sol::meta_function::division, sol::overload(
                sol::resolve<Vector2(float) const>(&Vector2::operator/),
                sol::resolve<Vector2(const Vector2&) const>(&Vector2::operator/)
            ),
            "divide", sol::overload(
                sol::resolve<Vector2&(float)>(&Vector2::operator/=),
                sol::resolve<Vector2&(const Vector2&)>(&Vector2::operator/=),
                [](Vector2& v, float _x, float _y) -> Vector2* {
                    v.x /= _x;
                    v.y /= _y;
                    return &v;
                }
            ),
            "cross", &Vector2::cross,
            "dot", &Vector2::dot,
            sol::meta_function::equal_to, &Vector2::operator==,
            sol::meta_function::to_string, [](const Vector2& v) {
                return std::string(v);
            },
            "string", [](const Vector2& v) {
                return std::string(v);
            }
        );
        lua.new_usertype<Vector3>("Vector3",
            sol::constructors<Vector3(), Vector3(float, float, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "z", &Vector3::z,
            sol::meta_function::addition, sol::overload(
                sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator+),
                sol::resolve<Vector3(const Vector2&) const>(&Vector3::operator+)
            ),
            "add", sol::overload(
                sol::resolve<Vector3&(const Vector3&)>(&Vector3::operator+=),
                sol::resolve<Vector3&(const Vector2&)>(&Vector3::operator+=),
                [](Vector3& v, float _x, float _y) -> Vector3* {
                    v.x += _x;
                    v.y += _y;
                    return &v;
                },
                [](Vector3& v, float _x, float _y, float _z) -> Vector3* {
                    v.x += _x;
                    v.y += _y;
                    v.z += _z;
                    return &v;
                } 
            ),
            "move", sol::overload(
                sol::resolve<Vector3&(const Vector3&)>(&Vector3::operator+=),
                sol::resolve<Vector3&(const Vector2&)>(&Vector3::operator+=),
                [](Vector3& v, float _x, float _y) -> Vector3* {
                    v.x += _x;
                    v.y += _y;
                    return &v;
                },
                [](Vector3& v, float _x, float _y, float _z) -> Vector3* {
                    v.x += _x;
                    v.y += _y;
                    v.z += _z;
                    return &v;
                } 
            ),
            sol::meta_function::subtraction, sol::overload(
                sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator-),
                sol::resolve<Vector3(const Vector2&) const>(&Vector3::operator-)
            ),
            "subtract", sol::overload(
                sol::resolve<Vector3&(const Vector3&)>(&Vector3::operator-=),
                sol::resolve<Vector3&(const Vector2&)>(&Vector3::operator-=),
                [](Vector3& v, float _x, float _y) -> Vector3* {
                    v.x -= _x;
                    v.y -= _y;
                    return &v;
                },
                [](Vector3& v, float _x, float _y, float _z) -> Vector3* {
                    v.x -= _x;
                    v.y -= _y;
                    v.z -= _z;
                    return &v;
                } 
            ),
            sol::meta_function::equal_to, &Vector3::operator==,
            sol::meta_function::to_string, [](const Vector3& v) {
                return std::string(v);
            },
            "string", [](const Vector3& v) {
                return std::string(v);
            }
        );
    }
}