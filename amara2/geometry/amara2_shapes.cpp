namespace Amara {
    struct Rectangle: public Vector2 {
        Rectangle() = default;
        Rectangle(float x_, float y_, float w_, float h_) : Vector2(x_, y_), w(w_), h(h_) {}
        Rectangle(SDL_FRect r) {
            x = static_cast<float>(r.x);
            y = static_cast<float>(r.y);
            w = static_cast<float>(r.w);
            h = static_cast<float>(r.h);
        }
        Rectangle(SDL_Rect r) {
            x = static_cast<float>(r.x);
            y = static_cast<float>(r.y);
            w = static_cast<float>(r.w);
            h = static_cast<float>(r.h);
        }
        Rectangle(nlohmann::json config) {
            *this = config;
        }
        Rectangle(sol::object obj) {
            *this = obj;
        }
            
        float w = 0;
        float h = 0;

        bool operator==(const Rectangle& other) const {
            return x == other.x && y == other.y && w == other.w && h == other.h;
        }
        bool operator !=(const Rectangle& other) const {
            return x != other.x || y != other.y || w != other.w || h != other.h;
        }
        
        explicit operator std::string() const {
            return "{ x: " + String::float_to_string(x) + ", y: " + String::float_to_string(y) + ", w: " + String::float_to_string(w) + ", h: " + String::float_to_string(h) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Rectangle& v) {
            return os << static_cast<std::string>(v);
        }

        static SDL_Rect makeSDLRect(const Rectangle& r) {
            SDL_Rect s = {
                static_cast<int>(r.x),
                static_cast<int>(r.y),
                static_cast<int>(r.w),
                static_cast<int>(r.h)
            };
            return s;
        }
        static SDL_FRect makeSDLFRect(const Rectangle& r) {
            SDL_FRect s = {
                r.x,
                r.y,
                r.w,
                r.h
            };
            return s;
        }

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y},
                {"w", w},
                {"h", h}
            });
        }

        Rectangle& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 4) {
                    x = config[0];
                    y = config[1];
                    w = config[2];
                    h = config[3];
                }
                else if (config.size() == 2) {
                    x = config[0];
                    y = config[1];
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];

                if (json_has(config, "w")) w = config["w"];
                else if (json_has(config, "width")) w = config["width"];
                
                if (json_has(config, "h")) h = config["h"];
                else if (json_has(config, "height")) h = config["height"];
            }
            return *this;
        }
        Rectangle& operator= (sol::object obj);

        Vector2 getCenter() {
            return Vector2(x + w/2, y + h/2);
        }
    };

    struct Quad {
        Quad() = default;
        Quad(const Rectangle& rect) {
            p1 = Vector2( rect.x, rect.y );
            p2 = Vector2( rect.x + rect.w, rect.y );
            p3 = Vector2( rect.x + rect.w, rect.y + rect.h );
            p4 = Vector2( rect.x, rect.y + rect.h );
        }
        Quad(const SDL_FRect& rect): Quad(Rectangle(rect)) {}
        Quad(const SDL_Rect& rect): Quad(Rectangle(rect)) {}
        Quad(
            const Vector2& _p1,
            const Vector2& _p2,
            const Vector2& _p3,
            const Vector2& _p4
        ) {
            p1 = _p1;
            p2 = _p2;
            p3 = _p3;
            p4 = _p4;
        }

        Vector2 p1;
        Vector2 p2;
        Vector2 p3;
        Vector2 p4;

        explicit operator std::string() const {
            return String::concat(
                "{ \n\t", std::string(p1), ",\n\t",
                std::string(p2), ",\n\t",
                std::string(p3), ",\n\t",
                std::string(p4), "\n}"
            );
        }
        friend std::ostream& operator<<(std::ostream& os, const Quad& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Circle: public Vector2 {
        Circle() = default;
        Circle(float x_, float y_, float r_) : Vector2(x_, y_), radius(r_) {}
        float radius = 0;

        Circle(const Vector2& v) : Vector2(v.x, v.y), radius(0) {}
        Circle(const SDL_FPoint& p) : Vector2(p.x, p.y), radius(0) {}
        Circle(const SDL_Point& p) : Vector2(p.x, p.y), radius(0) {}
        Circle(nlohmann::json config) {
            *this = config;
        }
        Circle(sol::object obj) {
            *this = obj;
        }

        bool operator==(const Circle& other) const {
            return x == other.x && y == other.y && radius == other.radius;
        }
        bool operator !=(const Circle& other) const {
            return x != other.x || y != other.y || radius != other.radius;
        }
        explicit operator std::string() const {
            return "{ x: " + String::float_to_string(x) + ", y: " + String::float_to_string(y) + ", radius: " + String::float_to_string(radius) + " }";
        }
        friend std::ostream& operator<<(std::ostream& os, const Circle& v) {
            return os << static_cast<std::string>(v);
        }

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"x", x},
                {"y", y},
                {"r", radius}
            });
        }
        Circle& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 3) {
                    x = config[0];
                    y = config[1];
                    radius = config[2];
                }
                else if (config.size() == 2) {
                    x = config[0];
                    y = config[1];
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];
                if (json_has(config, "r")) radius = config["r"];
                else if (json_has(config, "radius")) radius = config["radius"];
            }
            return *this;
        }
        Circle& operator= (sol::object obj);
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
            start = Vector2( x1, y1 );
            end = Vector2( x2, y2 );
        }
        Line(Vector2 _s, Vector2 _e): Line(_s.x, _s.y, _e.x, _e.y) {}

        Vector2 start = Vector2( 0, 0 );
        Vector2 end = Vector2( 0, 0 );
    };

    class Shape {
    public:
        using ShapeVariant = std::variant<
            Vector2, 
            Vector3,
            Rectangle,
            Quad,
            Circle, 
            Triangle, 
            Line
        >;

        ShapeVariant shape;

        Shape(): shape(Vector2( -1, -1 )) {}
        Shape(ShapeVariant _shape) {
            shape = _shape;
        }
        Shape(const Vector2& v) : shape(v) {}
        Shape(const Vector3& v) : shape(v) {}
        Shape(const Rectangle& r) : shape(r) {}
        Shape(const Quad& q) : shape(q) {}
        Shape(const Circle& c) : shape(c) {}
        Shape(const Triangle& t) : shape(t) {}
        Shape(const Line& l) : shape(l) {}

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
                return collision(s1, s2);
            }, shape, other.shape);
        }

        // static bool collision(const Rectangle& r1, const Rectangle& r2) {
        //     return (r1.w > 0 && r1.h > 0 && r2.w > 0 && r2.h > 0);
        // }
    
        // static bool collision(const Circle& c1, const Circle& c2) {
        //     double distance = std::abs(c1.radius - c2.radius);
        //     return distance < (c1.radius + c2.radius);
        // }
    
        // static bool collision(const Triangle& t1, const Triangle& t2) {
        //     return (t1.base > 0 && t1.height > 0 && t2.base > 0 && t2.height > 0);
        // }

        static bool collision(const Rectangle& r1, const Rectangle& r2) {
            if (r1.x + r1.w <= r2.x || r2.x + r2.w <= r1.x) {
                return false;
            }
            if (r1.y + r1.h <= r2.y || r2.y + r2.h <= r1.y) {
                return false;
            }
            return true;
        }

        static bool collision(const Quad& q1, const Quad& q2);
        static bool collision(const Circle& c1, const Circle& c2);
        static bool collision(const Vector2& p, const Quad& q);
        static bool collision(const Vector2& p, const Rectangle& r);
        static bool collision(const Rectangle& rect, const Quad& quad);
        
        template <typename T1, typename T2>
        static std::enable_if_t<!std::is_same_v<T1, T2>, bool> collision(const T1& a, const T2& b) {
            // Swap and call the correct function
            return collision(b, a);
        }
        
        template <typename T1, typename T2>
        static std::enable_if_t<std::is_same_v<T1, T2>, bool> collision(const T1&, const T2&) {
            return false;
        }
    };

    void bind_lua_Shapes(sol::state& lua) {
        lua.new_usertype<Rectangle>("Rectangle",
            sol::constructors<Rectangle(), Rectangle(float, float, float, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "w", &Rectangle::w,
            "h", &Rectangle::h,
            "width", &Rectangle::w,
            "height", &Rectangle::h,
            "string", [](const Rectangle& r) {
                return std::string(r);
            },
            "center", sol::property(&Rectangle::getCenter)
        );

        lua.new_usertype<Quad>("Quad",
            sol::constructors<Quad(), Quad(const Rectangle&)>(),
            "p1", sol::property(
                [](const Quad& q) { return q.p1; },
                [](Quad& q, sol::object v) { q.p1 = v; }
            ),
            "p2", sol::property(
                [](const Quad& q) { return q.p2; },
                [](Quad& q, sol::object v) { q.p2 = v; }
            ),
            "p3", sol::property(
                [](const Quad& q) { return q.p3; },
                [](Quad& q, sol::object v) { q.p3 = v; }
            ),
            "p4", sol::property(
                [](const Quad& q) { return q.p4; },
                [](Quad& q, sol::object v) { q.p4 = v; }
            ),
            "string", [](const Quad& q) {
                return std::string(q);
            }
        );

        lua.new_usertype<Circle>("Circle",
            sol::constructors<Circle(), Circle(float, float, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "radius", &Circle::radius
        );

        lua.new_usertype<Triangle>("Triangle",
            sol::constructors<Triangle(), Triangle(Vector2, Vector2, Vector2)>(),
            "p1", sol::property(
                [](const Triangle& t) { return t.p1; },
                [](Triangle& t, sol::object v) { t.p1 = v; }
            ),
            "p2", sol::property(
                [](const Triangle& t) { return t.p2; },
                [](Triangle& t, sol::object v) { t.p2 = v; }
            ),
            "p3", sol::property(
                [](const Triangle& t) { return t.p3; },
                [](Triangle& t, sol::object v) { t.p3 = v; }
            )
        );

        lua.new_usertype<Line>("Line",
            sol::constructors<Line(), Line(float, float, float, float), Line(Vector2, Vector2)>(),
            "start", sol::property(
                [](const Line& l) { return l.start; },
                [](Line& l, sol::object v) { l.start = v; }
            ),
            "end", sol::property(
                [](const Line& l) { return l.end; },
                [](Line& l, sol::object v) { l.end = v; }
            )
        );

        lua.new_usertype<Shape>("Shape",
            "collision",  sol::overload(
                [](const Rectangle& r1, const Rectangle& r2) {
                    return Shape::collision(r1, r2);
                },
                [](const Quad& q1, const Quad& q2) {
                    return Shape::collision(q1, q2);
                },
                [](const Circle& c1, const Circle& c2) {
                    return Shape::collision(c1, c2);
                },
                [](const Vector2& p, const Quad& q) {
                    return Shape::collision(p, q);
                },
                [](const Quad& q, const Vector2& p) {
                    return Shape::collision(p, q);
                },
                [](const Vector2& p, const Rectangle& r) {
                    return Shape::collision(p, r);
                },
                [](const Rectangle& r, const Vector2& p) {
                    return Shape::collision(p, r);
                },
                [](const Rectangle& rect, const Quad& quad) {
                    return Shape::collision(rect, quad);
                },
                [](const Quad& q, const Rectangle& r) {
                    return Shape::collision(r, q);
                }
            )
        );
    }
}