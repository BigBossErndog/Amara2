namespace Amara {
    float getDegrees(float radians) {
        return radians*180/M_PI;
    }
    float getRadians(float degrees) {
        return degrees*M_PI/180;
    }

    float distanceBetween(float sx, float sy, float ex, float ey) {
        float xDist = ex-sx;
        float yDist = ey-sy;
        return sqrt(xDist*xDist + yDist*yDist);
    }
    float distanceBetween(const Vector2& p1, const Vector2& p2) {
        return distanceBetween(p1.x, p1.y, p2.x, p2.y);
    }

    float angleBetween(float p1x, float p1y, float p2x, float p2y) {
        // Angle in Radians
        float angle = -atan2(p2y-p1y, p2x-p1x) + M_PI/2.0;
        while (angle < 0) {
            angle += 2*M_PI;
        }
        angle = fmod(angle, 2*M_PI);
        return angle;
    }
    float angleBetween(const Vector2& p1, const Vector2& p2) {
        // Angle in Radians
        return angleBetween(p1.x, p1.y, p2.x, p2.y);
    }

    Vector2 rotateAroundAnchor(const Vector2& v1, const Vector2& v2, float rotation) {
        // Rotate v2 around v1

        float s = sin(rotation);
        float c = cos(rotation);

        float dx = v2.x - v1.x;
        float dy = v2.y - v1.y;

        float newX = dx * c - dy * s;
        float newY = dx * s + dy * c;
        
        return { v1.x + newX, v1.y + newY };
    }
    Vector2 rotateAroundAnchor(const Vector2& v, float rotation) {
        return rotateAroundAnchor(Vector2(0, 0), v, rotation);
    }

    Vector2 centerOf(const Rectangle& rect) {
        return {
            static_cast<float>(rect.x + rect.w/2.0),
            static_cast<float>(rect.y + rect.h/2.0)
        };
    }

    Quad rotateQuad(const Quad& quad, const Vector2& anchor, float rotation) {
        return Quad(
            rotateAroundAnchor(anchor, quad.p1, rotation),
            rotateAroundAnchor(anchor, quad.p2, rotation),
            rotateAroundAnchor(anchor, quad.p3, rotation),
            rotateAroundAnchor(anchor, quad.p4, rotation)
        );
    }

    #ifdef AMARA_OPENGL
    Quad glTranslateQuad(const Rectangle& v, const Quad& q, bool insideTextureContainer) {
        if (!insideTextureContainer) {
            return {
                { -1.0f + (q.p1.x/v.w)*2, -1.0f + (1.0f - q.p1.y/v.h)*2 },
                { -1.0f + (q.p2.x/v.w)*2, -1.0f + (1.0f - q.p2.y/v.h)*2 },
                { -1.0f + (q.p3.x/v.w)*2, -1.0f + (1.0f - q.p3.y/v.h)*2 },
                { -1.0f + (q.p4.x/v.w)*2, -1.0f + (1.0f - q.p4.y/v.h)*2 }
            };
        }
        else {
            return {
                { -1.0f + (q.p1.x/v.w)*2, -1.0f + (q.p1.y/v.h)*2 },
                { -1.0f + (q.p2.x/v.w)*2, -1.0f + (q.p2.y/v.h)*2 },
                { -1.0f + (q.p3.x/v.w)*2, -1.0f + (q.p3.y/v.h)*2 },
                { -1.0f + (q.p4.x/v.w)*2, -1.0f + (q.p4.y/v.h)*2 }
            };
        }
    }
    #endif
    
    bool doIntersect(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) {
        auto orientation = [](const Vector2& a, const Vector2& b, const Vector2& c) {
            return (b - a).cross(c - a);
        };

        float o1 = orientation(p1, p2, p3);
        float o2 = orientation(p1, p2, p4);
        float o3 = orientation(p3, p4, p1);
        float o4 = orientation(p3, p4, p2);

        return (o1 * o2 < 0 && o3 * o4 < 0);
    }

    bool isPointInside(const Quad& quad, const Vector2& p) {
        auto sign = [](const Vector2& a, const Vector2& b, const Vector2& c) {
            return (b - a).cross(c - a);
        };

        bool b1 = sign(quad.p1, quad.p2, p) < 0.0f;
        bool b2 = sign(quad.p2, quad.p3, p) < 0.0f;
        bool b3 = sign(quad.p3, quad.p4, p) < 0.0f;
        bool b4 = sign(quad.p4, quad.p1, p) < 0.0f;

        return b1 == b2 && b2 == b3 && b3 == b4;
    }

    bool Shape::collision(const Quad& q1, const Quad& q2) {
        if (doIntersect(q1.p1, q1.p2, q2.p1, q2.p2) || doIntersect(q1.p1, q1.p2, q2.p2, q2.p3) ||
            doIntersect(q1.p1, q1.p2, q2.p3, q2.p4) || doIntersect(q1.p1, q1.p2, q2.p4, q2.p1) ||
            doIntersect(q1.p2, q1.p3, q2.p1, q2.p2) || doIntersect(q1.p2, q1.p3, q2.p2, q2.p3) ||
            doIntersect(q1.p2, q1.p3, q2.p3, q2.p4) || doIntersect(q1.p2, q1.p3, q2.p4, q2.p1) ||
            doIntersect(q1.p3, q1.p4, q2.p1, q2.p2) || doIntersect(q1.p3, q1.p4, q2.p2, q2.p3) ||
            doIntersect(q1.p3, q1.p4, q2.p3, q2.p4) || doIntersect(q1.p3, q1.p4, q2.p4, q2.p1) ||
            doIntersect(q1.p4, q1.p1, q2.p1, q2.p2) || doIntersect(q1.p4, q1.p1, q2.p2, q2.p3) ||
            doIntersect(q1.p4, q1.p1, q2.p3, q2.p4) || doIntersect(q1.p4, q1.p1, q2.p4, q2.p1)) {
            return true;
        }
        if (isPointInside(q2, q1.p1) || isPointInside(q2, q1.p2) || isPointInside(q2, q1.p3) || isPointInside(q2, q1.p4)) {
            return true;
        }
        if (isPointInside(q1, q2.p1) || isPointInside(q1, q2.p2) || isPointInside(q1, q2.p3) || isPointInside(q1, q2.p4)) {
            return true;
        }

        return false;
    }

    bool Shape::collision(const Circle& c1, const Circle& c2) {
        double distance = distanceBetween(c1.x, c1.y, c2.x, c2.y);
        if (distance <= (c1.radius + c2.radius)) return true;
    }

    bool Shape::collision(const Vector2& p, const Quad& q) {
        return isPointInside(q, p);
    }

    bool Shape::collision(const Vector2& p, const Rectangle& r) {
        return (p.x >= r.x && p.x <= r.x + r.w &&
                p.y >= r.y && p.y <= r.y + r.h &&
                r.w > 0 && r.h > 0);
    }

    bool Shape::collision(const Rectangle& rect, const Quad& quad) {
        return Shape::collision(quad, Quad(rect));
    }

    Vector2 stringToPosition(std::string str) {
        if (String::equal(str, "top")) return { 0.5, 0 };
        if (String::equal(str, "bottom")) return { 0.5, 1 };
        if (String::equal(str, "left")) return { 0, 0.5 };
        if (String::equal(str, "right")) return { 1, 0.5 };
        if (String::equal(str, "center")) return { 0.5, 0.5 };
        if (String::equal(str, "topLeft")) return { 0, 0 };
        if (String::equal(str, "topRight")) return { 1, 0 };
        if (String::equal(str, "bottomLeft")) return { 0, 1 };
        if (String::equal(str, "bottomRight")) return { 1, 1 };
        return { 0.5, 0.5 };
    }
    
    Vector2& Vector2::operator= (nlohmann::json config) {
        if (config.is_string()) *this = stringToPosition(config.get<std::string>());
        else if (config.is_number()) {
            x = y = config.get<float>();
        }
        else if (config.is_object()) {
            if (json_has(config, "x")) x = config["x"];
            if (json_has(config, "y")) y = config["y"];
        }
        else if (config.is_array()) {
            if (config.size() == 2) {
                x = config[0];
                y = config[1];
            }
        }
        return *this;
    }

    void bind_lua_Geometry(sol::state& lua) {
        sol::table math_metatable = lua["math"];
        math_metatable.set_function("rotateAroundAnchor", sol::overload(
            sol::resolve<Vector2(const Vector2&, const Vector2&, float)>(&Amara::rotateAroundAnchor),
            sol::resolve<Vector2(const Vector2&, float)>(&Amara::rotateAroundAnchor)
        ));
        math_metatable.set_function("distanceBetween", sol::overload(
            sol::resolve<float(float, float, float, float)>(&Amara::distanceBetween),
            sol::resolve<float(const Vector2&, const Vector2&)>(&Amara::distanceBetween)
        ));
        math_metatable.set_function("angleBetween", sol::overload(
            sol::resolve<float(float, float, float, float)>(&Amara::angleBetween),
            sol::resolve<float(const Vector2&, const Vector2&)>(&Amara::angleBetween)
        ));
        math_metatable.set_function("centerOf", &Amara::centerOf);

        lua.new_enum("Position",
            "Top", &Vector2::Top,
            "Bottom", &Vector2::Bottom,
            "Left", &Vector2::Left,
            "Right", &Vector2::Right,
            "Center", &Vector2::Center,
            "TopLeft", &Vector2::TopLeft,
            "TopRight", &Vector2::TopRight,
            "BottomLeft", &Vector2::BottomLeft,
            "BottomRight", &Vector2::BottomRight
        );
    }
}