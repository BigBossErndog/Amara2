namespace Amara {
    struct Rectangle: public Vector2 {
        Rectangle() = default;
        Rectangle(float x_, float y_, float w_, float h_) : Vector2(x_, y_), w(w_), h(h_) {}

        float w = 0;
        float h = 0;

        bool operator==(const Rectangle& other) const {
            return x == other.x && y == other.y && w == other.w && h == other.h;
        }
        
        explicit operator std::string() const {
            return "{ x: " + float_string(x) + ", y: " + float_string(y) + ", w: " + float_string(w) + ", h: " + float_string(h) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Rectangle& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Circle: public Vector2 {
        Circle() = default;
        Circle(float x_, float y_, float r_) : Vector2(x_, y_), radius(r_) {}
        float radius = 0;
    };
    
    struct Triangle {
        Triangle() = default;

        Triangle(Vector2 _p1, Vector2 _p2, Vector2 _p3) {
            p1 = _p1;
            p2 = _p2;
            p3 = _p3;
        }

        Vector2 p1 = {0, 0};
        Vector2 p2 = {0, 0};
        Vector2 p3 = {0, 0};
    };

    struct Line {
        // From point to point
        Line() = default;
        Line(float x1, float y1, float x2, float y2) {
            start = { x1, y1 };
            end = { x2, y2 };
        }
        Line(Vector2 _s, Vector2 _e): Line(_s.x, _s.y, _e.x, _e.y) {}

        Vector2 start = {0, 0};
        Vector2 end = {0, 0};
    };

    class Shape {
    private:
        using ShapeVariant = std::variant<
            Vector2, 
            Vector3, 
            Rectangle, 
            Circle, 
            Triangle, 
            Line
        >;

        ShapeVariant shape;
        
    public:

        Shape(): shape(Vector2()) {}

        template <typename T>
        operator T() const {
            if (std::holds_alternative<T>(shape)) 
                return std::get<T>(shape);
            throw std::bad_cast();
        }
        
        template <typename T>
        Shape& operator=(const T& newShape) {
            shape = newShape;
            return *this;
        }

        template <typename T>
        bool is() const {
            return std::holds_alternative<T>(shape);
        }

        template <typename T>
        T as() const {
            if (!std::holds_alternative<T>(shape))
                throw std::bad_cast();
            return std::get<T>(shape);
        }
        
        bool collidesWith(const Shape& other) const {
            return std::visit([](const auto& s1, const auto& s2) {
                return checkCollision(s1, s2);
            }, shape, other.shape);
        }

        // static bool checkCollision(const Rectangle& r1, const Rectangle& r2) {
        //     return (r1.w > 0 && r1.h > 0 && r2.w > 0 && r2.h > 0);
        // }
    
        // static bool checkCollision(const Circle& c1, const Circle& c2) {
        //     double distance = std::abs(c1.radius - c2.radius);
        //     return distance < (c1.radius + c2.radius);
        // }
    
        // static bool checkCollision(const Triangle& t1, const Triangle& t2) {
        //     return (t1.base > 0 && t1.height > 0 && t2.base > 0 && t2.height > 0);
        // }

        template <typename T1, typename T2>
        static std::enable_if_t<!std::is_same_v<T1, T2>, bool> checkCollision(const T1& a, const T2& b) {
            // Swap and call the correct function
            return checkCollision(b, a);
        }
        
        template <typename T1, typename T2>
        static std::enable_if_t<std::is_same_v<T1, T2>, bool> checkCollision(const T1&, const T2&) {
            return false;
        }
    };

    void bindLua_Shapes(sol::state& lua) {
        lua.new_usertype<Rectangle>("Rectangle",
            sol::constructors<Rectangle(), Rectangle(float, float, float, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "w", &Rectangle::w,
            "h", &Rectangle::h,
            "string", [](const Rectangle& r) {
                return std::string(r);
            }
        );
    }
}