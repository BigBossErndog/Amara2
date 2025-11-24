namespace Amara {
    class Shape;

    nlohmann::json lua_to_json(sol::object obj);

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

        float diagonal() {
            return sqrt(w*w + h*h);
        }
        float area() {
            return w * h;
        }
        float aspectRatio() {
            return w / h;
        }
        float perimeter() {
            return 2 * (w + h);
        }

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
                else {
                    fatal_error("Error: Invalid Rectangle assignment.");
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];

                if (json_has(config, "w")) w = config["w"];
                else if (json_has(config, "width")) w = config["width"];
                
                if (json_has(config, "h")) h = config["h"];
                else if (json_has(config, "height")) h = config["height"];

                if (!json_has_any(config, "w", "h", "width", "height")) {
                    fatal_error("Error: Invalid Rectangle assignment.");
                }
            }
            else {
                fatal_error("Error: Invalid Rectangle assignment.");
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

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"p1", p1.toJSON()},
                {"p2", p2.toJSON()},
                {"p3", p3.toJSON()},
                {"p4", p4.toJSON()}
            });
        }

        Quad& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 4) {
                    p1 = config[0];
                    p2 = config[1];
                    p3 = config[2];
                    p4 = config[3];
                }
                else if (config.size() == 8) {
                    p1 = Vector2(config[0], config[1]);
                    p2 = Vector2(config[2], config[3]);
                    p3 = Vector2(config[4], config[5]);
                    p4 = Vector2(config[6], config[7]);
                }
                else {
                    fatal_error("Error: Invalid Quad assignment.");
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "p1")) p1 = config["p1"];
                if (json_has(config, "p2")) p2 = config["p2"];
                if (json_has(config, "p3")) p3 = config["p3"];
                if (json_has(config, "p4")) p4 = config["p4"];
            }
            else {
                fatal_error("Error: Invalid Quad assignment.");
            }
            return *this;
        }
    };

    struct Circle: public Vector2 {
        float radius = 0;

        Circle() = default;
        Circle(float r_) : radius(r_) {}
        Circle(float x_, float y_, float r_) : Vector2(x_, y_), radius(r_) {}

        Circle(const Vector2& v) : Vector2(v.x, v.y), radius(0) {}
        Circle(const Vector2& v, float r) : Vector2(v.x, v.y), radius(r) {}
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
                else {
                    fatal_error("Error: Invalid Circle assignment.");
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "x")) x = config["x"];
                if (json_has(config, "y")) y = config["y"];
                if (json_has(config, "r")) radius = config["r"];
                else if (json_has(config, "radius")) radius = config["radius"];
            }
            else {
                fatal_error("Error: Invalid Circle assignment.");
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
        Triangle(nlohmann::json config) {
            *this = config;
        }
        Triangle(sol::object obj) {
            *this = obj;
        }

        Vector2 p1 = {0, 0};
        Vector2 p2 = {0, 0};
        Vector2 p3 = {0, 0};

        Triangle& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 3) {
                    p1 = config[0];
                    p2 = config[1];
                    p3 = config[2];
                }
                else {
                    fatal_error("Error: Invalid Triangle assignment.");
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "p1")) p1 = config["p1"];
                if (json_has(config, "p2")) p2 = config["p2"];
                if (json_has(config, "p3")) p3 = config["p3"];
                if (!json_has_any(config, "p1", "p2", "p3")) {
                    fatal_error("Error: Invalid Triangle assignment.");
                }
            }
            else {
                fatal_error("Error: Invalid Triangle assignment.");
        
            }
            return *this;
        }
        Triangle& operator= (sol::object obj);

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"p1", p1.toJSON()},
                {"p2", p2.toJSON()},
                {"p3", p3.toJSON()}
            });
        }

        explicit operator std::string() const {
            return String::concat("{ p1: ", std::string(p1), ", p2: ", std::string(p2), ", p3: ", std::string(p3), " }");
        }
        friend std::ostream& operator<<(std::ostream& os, const Triangle& v) {
            return os << static_cast<std::string>(v);
        }
    };

    struct Line {
        // From point to point
        Line() = default;
        Line(float x1, float y1, float x2, float y2) {
            start = Vector2( x1, y1 );
            end = Vector2( x2, y2 );
        }
        Line(Vector2 _s, Vector2 _e): Line(_s.x, _s.y, _e.x, _e.y) {}
        Line(nlohmann::json config) {
            *this = config;
        }
        Line(sol::object obj) {
            *this = obj;
        }

        Vector2 start = Vector2( 0, 0 );
        Vector2 end = Vector2( 0, 0 );

        std::vector<Vector2> split(int num) {
            std::vector<Vector2> points;
            for (int i = 0; i <= (num-1); i++) {
                points.push_back(Vector2(getPoint(i / (num-1))));
            }
            return points;
        }

        Vector2 getPoint(float t) {
            return Vector2(
                start.x + (end.x - start.x) * t,
                start.y + (end.y - start.y) * t
            );
        }

        Line& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 4) {
                    start = Vector2(config[0], config[1]);
                    end = Vector2(config[2], config[3]);
                }
                else if (config.size() == 2) {
                    start = Vector2(config[0]);
                    end = Vector2(config[1]);
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "start")) start = config["start"];
                if (json_has(config, "end")) end = config["end"];
                if (json_has(config, "p1")) start = config["p1"];
                if (json_has(config, "p2")) end = config["p2"];
            }
            return *this;
        }
        Line& operator= (sol::object obj);

        nlohmann::json toJSON() {
            return nlohmann::json::object({
                {"start", start.toJSON()},
                {"end", end.toJSON()}
            });
        }

        explicit operator std::string() const {
            return String::concat("{ start: ", std::string(start), ", end: ", std::string(end), " }");
        }
        friend std::ostream& operator<<(std::ostream& os, const Line& v) {
            return os << static_cast<std::string>(v);
        }
    };

    class CustomCollider {
    public:
        CustomCollider() = default;

        virtual bool collidesWithShape(const Shape& other) { return false; };
    };
    struct CustomShape {
        CustomShape() = default;
        CustomShape(CustomCollider* _collider) {
            collider = _collider;
        }

        CustomCollider* collider = nullptr;
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
            Line,
            std::vector<Shape>,
            CustomShape
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
        Shape(const std::vector<Shape>& list) : shape(list) {}
        Shape(const CustomShape& c) : shape(c) {}

        Shape(const nlohmann::json& config) {
            *this = config;
        }
        Shape(sol::object obj) {
            *this = obj;
        }

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

        Shape move(const Vector2& v) {
            if (is<Rectangle>()) {
                Rectangle r = as<Rectangle>();
                r.x += v.x;
                r.y += v.y;
                return r;
            }
            else if (is<Quad>()) {
                Quad q = as<Quad>();
                q.p1.x += v.x;
                q.p1.y += v.y;
                q.p2.x += v.x;
                q.p2.y += v.y;
                q.p3.x += v.x;
                q.p3.y += v.y;
                q.p4.x += v.x;
                q.p4.y += v.y;

                return q;
            }
            else if (is<Circle>()) {
                Circle c = as<Circle>();
                c.x += v.x;
                c.y += v.y;

                return c;
            }
            else if (is<Triangle>()) {
                Triangle t = as<Triangle>();
                t.p1.x += v.x;
                t.p1.y += v.y;
                t.p2.x += v.x;
                t.p2.y += v.y;
                t.p3.x += v.x;
                t.p3.y += v.y;

                return t;
            }
            else if (is<Line>()) {
                Line l = as<Line>();
                l.start.x += v.x;
                l.start.y += v.y;
                l.end.x += v.x;
                l.end.y += v.y;

                return l;
            }
            else if (is<Vector3>()) {
                Vector3 v3 = as<Vector3>();
                v3.x += v.x;
                v3.y += v.y;

                return v;
            }
            else if (is<Vector2>()) {
                Vector2 v2 = as<Vector2>();
                v2.x += v.x;
                v2.y += v.y;

                return v;
            }
            return *this;
        }

        template <typename T1, typename T2>
        static bool checkCollision(const T1& s1, const T2& s2);
        
        bool collidesWith(const Shape& other) const;
        
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
        static bool collision(const Triangle& t1, const Triangle& t2);
        static bool collision(const Line& l1, const Line& l2);

        static bool collision(const Vector2& p, const Quad& q);
        static bool collision(const Vector2& p, const Rectangle& r);

        static bool collision(const Rectangle& rect, const Quad& quad);
        static bool collision(const Rectangle& rect, const Circle& circle);
        static bool collision(const Rectangle& rect, const Triangle& triangle);

        static bool collision(const Quad& q, const Circle& r);
        static bool collision(const Quad& q, const Triangle& t);

        static bool collision(const Shape& s1, const std::vector<Shape>& list) {
            for (const auto& s2 : list) {
                if (s1.collidesWith(s2)) {
                    return true;
                }
            }
            return false;
        }
        static bool collision(const std::vector<Shape>& list, const Shape& s2) {
            return collision(s2, list);
        }

        Vector2 getCenter() {
            if (is<Rectangle>()) {
                return as<Rectangle>().getCenter();
            }
            else if (is<Quad>()) {
                return as<Quad>().p1 + as<Quad>().p2 + as<Quad>().p3 + as<Quad>().p4 / 4;
            }
            else if (is<Circle>()) {
                return as<Circle>();
            }
            else if (is<Triangle>()) {
                return as<Triangle>().p1 + as<Triangle>().p2 + as<Triangle>().p3 / 3;
            }
            else if (is<Line>()) {
                return as<Line>().start + as<Line>().end / 2;
            }
            else if (is<Vector2>()) {
                return as<Vector2>();
            }
            else if (is<Vector3>()) {
                return (Vector2)as<Vector3>();
            }
            return Vector2( 0, 0 );
        }

        Shape& operator= (const nlohmann::json& config) {
            if (config.is_array()) {
                if (config.size() == 2) {
                    if (config[0].is_number()) {
                        shape = Vector2(config);
                    }
                    else if (config[0].is_object() || config[0].is_array()) {
                        if (config[1].is_number()) {
                            shape = Circle(config);
                        }
                        else {
                            shape = Line(config);
                        }
                    }
                }
                else if (config.size() == 3) {
                    if (config[0].is_number()) {
                        shape = Vector3(config);
                    }
                    else if (config[0].is_object() || config[0].is_array()) {
                        shape = Triangle(config);
                    }
                }
                else if (config.size() == 4) {
                    if (config[0].is_number()) {
                        shape = Rectangle(config);
                    }
                    else {
                        shape = Quad(config);
                    }
                }
            }
            else if (config.is_object()) {
                if (json_has(config, "x", "y", "w", "h")) {
                    shape = Rectangle(config);
                }
                else if (json_has(config, "x", "y", "width", "height")) {
                    shape = Rectangle(config);
                }
                else if (json_has(config, "x", "y", "z")) {
                    shape = Vector3(config["x"], config["y"], config["z"]);
                }
                else if (json_has(config, "x", "y", "r")) {
                    shape = Circle(config);
                }
                else if (json_has(config, "x", "y", "radius")) {
                    shape = Circle(config);
                }
                else if (json_has(config, "x", "y")) {
                    shape = Vector2(config["x"], config["y"]);
                }
                else if (json_has(config, "p1", "p2", "p3", "p4")) {
                    shape = Quad(config);
                }
                else if (json_has(config, "p1", "p2", "p3")) {
                    shape = Triangle(config);
                }
                else if (json_has(config, "p1", "p2")) {
                    shape = Line(config);
                }
                else if (json_has(config, "start", "end")) {
                    shape = Line(config);
                }
            }
            return *this;
        }

        Shape& operator= (sol::object obj) {
            if (obj.is<Rectangle>()) {
                shape = obj.as<Rectangle>();
            }
            else if (obj.is<Quad>()) {
                shape = obj.as<Quad>();
            }
            else if (obj.is<Circle>()) {
                shape = obj.as<Circle>();
            }
            else if (obj.is<Triangle>()) {
                shape = obj.as<Triangle>();
            }
            else if (obj.is<Line>()) {
                shape = obj.as<Line>();
            }
            else if (obj.is<Vector2>()) {
                shape = obj.as<Vector2>();
            }
            else if (obj.is<Vector3>()) {
                shape = obj.as<Vector3>();
            }
            else if (obj.is<std::vector<Shape>>()) {
                shape = obj.as<std::vector<Shape>>();
            }
            else if (obj.is<sol::table>()) {
                sol::table t = obj.as<sol::table>();
                *this = lua_to_json(obj);
            }
            
            return *this;
        }
        
        sol::object get_lua_object(sol::state& lua) {
            if (is<Rectangle>()) {
                return sol::make_object(lua, as<Rectangle>());
            }
            else if (is<Quad>()) {
                return sol::make_object(lua, as<Quad>());
            }
            else if (is<Circle>()) {
                return sol::make_object(lua, as<Circle>());
            }
            else if (is<Triangle>()) {
                return sol::make_object(lua, as<Triangle>());
            }
            else if (is<Line>()) {
                return sol::make_object(lua, as<Line>());
            }
            else if (is<Vector2>()) {
                return sol::make_object(lua, as<Vector2>());
            }
            else if (is<Vector3>()) {
                return sol::make_object(lua, as<Vector3>());
            }
            return sol::nil;
        }
        
        explicit operator std::string() const {
            return std::visit([](const auto& s) -> std::string {
                using T = std::decay_t<decltype(s)>;
                if constexpr (
                    std::is_same_v<T, Vector2> ||
                    std::is_same_v<T, Vector3> ||
                    std::is_same_v<T, Rectangle> ||
                    std::is_same_v<T, Quad> ||
                    std::is_same_v<T, Circle> ||
                    std::is_same_v<T, Triangle> ||
                    std::is_same_v<T, Line>
                ) {
                    return std::string(s);
                }
                else if constexpr (std::is_same_v<T, std::vector<Shape>>) {
                    std::string result = "[";
                    bool first = true;
                    for (const auto& shape : s) {
                        if (!first) {
                            result += ", ";
                        }
                        result += std::string(shape);
                        first = false;
                    }
                    result += "]";
                    return result;
                }
                return "Unknown Shape";
            }, shape);
        }
        friend std::ostream& operator<<(std::ostream& os, const Shape& v) {
            return os << static_cast<std::string>(v);
        }
    };
    
    template <typename T1, typename T2, typename = void>
    struct has_collision_overload : std::false_type {};

    template <typename T1, typename T2>
    struct has_collision_overload<
        T1, T2,
        std::void_t<decltype(Shape::collision(std::declval<T1>(), std::declval<T2>()))>>
        : std::true_type {};

    template <typename T1, typename T2>
    constexpr bool has_collision_overload_v = has_collision_overload<T1, T2>::value;

    namespace detail {
        template <typename T1, typename T2>
        bool do_collision(const T1& s1, const T2& s2) {
            return Shape::collision(s1, s2);
        }
    }

    template <typename T1, typename T2>
    bool Shape::checkCollision(const T1& s1, const T2& s2) {
        if constexpr (has_collision_overload_v<T1, T2>) {
            return detail::do_collision(s1, s2);
        } else if constexpr (has_collision_overload_v<T2, T1>) {
            return detail::do_collision(s2, s1);
        } else {
            return false;
        }
    }

    bool Shape::collidesWith(const Shape& other) const {
        if (is<std::vector<Shape>>()) {
            return collision(as<std::vector<Shape>>(), other);
        }
        if (other.is<std::vector<Shape>>()) {
            return collision(other.as<std::vector<Shape>>(), *this);
        }
        if (is<CustomShape>()) {
            const CustomShape& custom = as<CustomShape>();
            if (custom.collider) {
                return custom.collider->collidesWithShape(other);
            }
            return false;
        }
        if (other.is<CustomShape>()) {
            const CustomShape& custom = other.as<CustomShape>();
            if (custom.collider) {
                return custom.collider->collidesWithShape(*this);
            }
            return false;
        }
        return std::visit([](const auto& s1, const auto& s2) -> bool {
            return checkCollision(s1, s2);
        }, shape, other.shape);
    }

    void bind_lua_Shapes(sol::state& lua) {
        lua.new_usertype<Rectangle>("Rectangle",
            sol::constructors<Rectangle(), Rectangle(float, float, float, float)>(),
            sol::base_classes, sol::bases<Vector2>(),
            "w", &Rectangle::w,
            "h", &Rectangle::h,
            "width", &Rectangle::w,
            "height", &Rectangle::h,
            "left", sol::property([](Amara::Rectangle& r) { return r.x; }),
            "top", sol::property([](Amara::Rectangle& r) { return r.y; }),
            "right", sol::property([](Amara::Rectangle& r) { return r.x + r.w; }),
            "bottom", sol::property([](Amara::Rectangle& r) { return r.y + r.h; }),
            "string", [](const Rectangle& r) {
                return std::string(r);
            },
            "center", sol::property(&Rectangle::getCenter),
            "diagonal", sol::property(&Rectangle::diagonal),
            "area", sol::property(&Rectangle::area),
            "perimeter", sol::property(&Rectangle::perimeter)
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
            sol::constructors<Circle(), Circle(float), Circle(float, float, float), Circle(Vector2), Circle(Vector2, float)>(),
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
            ),
            "split", [](Line& l, sol::object _n) {
                if (_n.is<int>()) {
                    return l.split(_n.as<int>());
                }
                if (_n.is<double>()) {
                    return l.split(static_cast<int>(_n.as<double>()));
                }
                return std::vector<Vector2>();
            }
        );

        lua.new_usertype<Shape>("shape",
            "collision",  sol::overload(
                [](const Rectangle& r1, const Rectangle& r2) {
                    return Shape(r1).collidesWith(r2);
                },
                [](const Quad& q1, const Quad& q2) {
                    return Shape(q1).collidesWith(q2);
                },
                [](const Circle& c1, const Circle& c2) {
                    return Shape(c1).collidesWith(c2);
                },
                [](const Vector2& p, const Quad& q) {
                    return Shape(p).collidesWith(q);
                },
                [](const Quad& q, const Vector2& p) {
                    return Shape(p).collidesWith(q);
                },
                [](const Vector2& p, const Rectangle& r) {
                    return Shape(p).collidesWith(r);
                },
                [](const Rectangle& r, const Vector2& p) {
                    return Shape(p).collidesWith(r);
                },
                [](const Rectangle& rect, const Quad& quad) {
                    return Shape(rect).collidesWith(quad);
                },
                [](const Quad& q, const Rectangle& r) {
                    return Shape(r).collidesWith(q);
                }
            )
        );
    }
}