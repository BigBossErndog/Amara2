namespace Amara {    
    struct Vector2 {
        Vector2() = default;
        Vector2(float x_, float y_) : x(x_), y(y_) {}

        float x = 0;
        float y = 0;

        Vector2 operator+ (const Vector2& other) const {
            return Vector2(x + other.x, y + other.y);
        }
        Vector2& operator+=(const Vector2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2 operator- (const Vector2& other) const {
            return Vector2(x - other.x, y - other.y);
        }
        Vector2& operator-=(const Vector2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        bool operator==(const Vector2& other) const {
            return x == other.x && y == other.y;
        }

        explicit operator std::string() const {
            return "{ x: " + float_string(x) + ", y: " + float_string(y) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Vector3: public Vector2 {
        Vector3() = default;
        Vector3(float x_, float y_, float z_) : Vector2(x_, y_), z(z_) {}

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