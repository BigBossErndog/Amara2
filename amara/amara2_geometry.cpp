namespace Amara {
    typedef struct IntVector2 {
        int x = 0;
        int y = 0;
    } IntVector2;

    class IntVector3: public IntVector2 {
        public: int z = 0;
    };
    
    struct FloatVector2 {
        FloatVector2() = default;
        FloatVector2(float x_, float y_) : x(x_), y(y_) {}

        float x = 0;
        float y = 0;
    };

    struct FloatVector3: public FloatVector2 {
        FloatVector3() = default;
        FloatVector3(float x_, float y_, float z_) : FloatVector2(x_, y_), z(z_) {}

        public: float z = 0;
    };

    struct IntRect: public IntVector2 {
        int width = 0;
        int height = 0;
    };

    struct FloatRect: public FloatVector2 {
        float width = 0;
        float height = 0;
    };

    struct FloatCircle: public FloatVector2 {
        float radius = 0;
    };

    struct FloatLine {
        FloatVector2 p1 = {0, 0};
        FloatVector2 p2 = {0, 0};
    };

    struct FloatTriangle {
        FloatVector2 p1 = {0, 0};
        FloatVector2 p2 = {0, 0};
        FloatVector2 p3 = {0, 0};
    };

    void bindLuaGeometry(sol::state& lua) {
        lua.new_usertype<FloatVector2>("FloatVector2",
            sol::constructors<FloatVector2(), FloatVector2(float, float)>(),
            "x", &FloatVector2::x,
            "y", &FloatVector2::y,
            sol::meta_function::to_string, [](const FloatVector2& v) {
                return std::string("( x: ") +
                       std::to_string(v.x) + ", y: " +
                       std::to_string(v.y) + ")";
            }
        );
        lua.new_usertype<FloatVector3>("FloatVector3",
            sol::constructors<FloatVector3(), FloatVector3(float, float, float)>(),
            sol::base_classes, sol::bases<FloatVector2>(),
            "z", &FloatVector3::z,
            sol::meta_function::to_string, [](const FloatVector3& v) {
                return std::string("( x: ") +
                       std::to_string(v.x) + ", y: " +
                       std::to_string(v.y) + ", z: " +
                       std::to_string(v.z) + ")";
            }
        );
    }
}