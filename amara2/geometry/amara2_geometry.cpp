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

    float closestEquivalentAngle(float angle1, float angle2) {
        angle2 = std::remainder(angle2, 2.0 * M_PI);
        float baseAngle = floor(angle1/(2.0*M_PI))*(2.0*M_PI);
        float check1 = baseAngle + angle2;
        float check2 = check1 + ((check1 < angle1) ? (2.0*M_PI) : (-2.0*M_PI));

        return (fabs(check1 - angle1) < fabs(check2 - angle1)) ? check1 : check2;
    }

    float angleDifference(float angle1, float angle2) {
        
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

    Quad moveQuad(const Quad& quad, const Vector2& offset) {
        return Quad(
            quad.p1 + offset,
            quad.p2 + offset,
            quad.p3 + offset,
            quad.p4 + offset
        );
    }
    Quad moveQuad(const Quad& quad, float offsetX, float offsetY) {
        return moveQuad(quad, Vector2(offsetX, offsetY));
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
        Vector2 verts[4] = {quad.p1, quad.p2, quad.p3, quad.p4};
        bool inside = false;

        for (int i = 0, j = 3; i < 4; j = i++) {
            const Vector2& a = verts[i];
            const Vector2& b = verts[j];

            bool crossesY = ((a.y > p.y) != (b.y > p.y));
            if (crossesY) {
                float xAtPy = a.x + (b.x - a.x) * (p.y - a.y) / ((b.y - a.y) + 1e-6f);
                if (p.x < xAtPy) inside = !inside;
            }
        }
        return inside;
    }

    bool isPointInside(const Triangle& triangle, const Vector2& p) {
        auto sign = [](const Vector2& p1, const Vector2& p2, const Vector2& p3) {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };

        float d1, d2, d3;
        bool has_neg, has_pos;

        d1 = sign(p, triangle.p1, triangle.p2);
        d2 = sign(p, triangle.p2, triangle.p3);
        d3 = sign(p, triangle.p3, triangle.p1);

        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }

    bool isPointInside(const Circle& circle, const Vector2& p) {
        return distanceBetween(circle, p) <= circle.radius;
    }

    bool Shape::collision(const Quad& q1, const Quad& q2) {
        auto edgeNormal = [](const Vector2& a, const Vector2& b) {
            Vector2 edge = { b.x - a.x, b.y - a.y };
            return Vector2{ -edge.y, edge.x }; // perpendicular
        };

        auto project = [&](const Quad& q, const Vector2& axis, float& min, float& max) {
            min = max = q.p1.dot(axis);
            float d2 = q.p2.dot(axis);
            float d3 = q.p3.dot(axis);
            float d4 = q.p4.dot(axis);
            min = std::min({min, d2, d3, d4});
            max = std::max({max, d2, d3, d4});
        };

        auto overlapOnAxis = [&](const Vector2& axis) {
            float min1, max1, min2, max2;
            project(q1, axis, min1, max1);
            project(q2, axis, min2, max2);
            return !(max1 < min2 || max2 < min1);
        };

        Vector2 axes[8] = {
            edgeNormal(q1.p1, q1.p2),
            edgeNormal(q1.p2, q1.p3),
            edgeNormal(q1.p3, q1.p4),
            edgeNormal(q1.p4, q1.p1),
            edgeNormal(q2.p1, q2.p2),
            edgeNormal(q2.p2, q2.p3),
            edgeNormal(q2.p3, q2.p4),
            edgeNormal(q2.p4, q2.p1)
        };

        for (const Vector2& axis : axes) {
            if (!overlapOnAxis(axis)) {
                return false;
            }
        }
        return true;
    }

    bool Shape::collision(const Circle& c1, const Circle& c2) {
        double distance = distanceBetween(c1.x, c1.y, c2.x, c2.y);
        if (distance <= (c1.radius + c2.radius)) return true;
    }

    bool Shape::collision(const Triangle& t1, const Triangle& t2) {
        auto getAxes = [](const Vector2* vertices, int num_vertices) {
            std::vector<Vector2> axes;
            for (int i = 0; i < num_vertices; i++) {
                Vector2 edge = vertices[(i + 1) % num_vertices] - vertices[i];
                Vector2 axis(-edge.y, edge.x); // perpendicular
                float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
                if (len > 1e-6f) {
                    axis.x /= len;
                    axis.y /= len;
                    axes.push_back(axis);
                }
            }
            return axes;
        };

        auto project = [](const Vector2* vertices, int num_vertices, const Vector2& axis) {
            float min = vertices[0].dot(axis);
            float max = min;
            for (int i = 1; i < num_vertices; i++) {
                float p = vertices[i].dot(axis);
                if (p < min) min = p;
                else if (p > max) max = p;
            }
            return std::make_pair(min, max);
        };

        Vector2 t1v[] = { t1.p1, t1.p2, t1.p3 };
        Vector2 t2v[] = { t2.p1, t2.p2, t2.p3 };

        std::vector<Vector2> axes = getAxes(t1v, 3);
        std::vector<Vector2> t2Axes = getAxes(t2v, 3);
        axes.insert(axes.end(), t2Axes.begin(), t2Axes.end());

        const float EPS = 1e-6f;
        for (const auto& axis : axes) {
            auto p1 = project(t1v, 3, axis);
            auto p2 = project(t2v, 3, axis);
            if (p1.second < p2.first - EPS || p2.second < p1.first - EPS) {
                return false; // separation found
            }
        }

        auto isPointInsideTriangle = [](const Triangle& tri, const Vector2& p) {
            auto sign = [](const Vector2& a, const Vector2& b, const Vector2& c) {
                return (a.x - c.x)*(b.y - c.y) - (b.x - c.x)*(a.y - c.y);
            };
            float d1 = sign(p, tri.p1, tri.p2);
            float d2 = sign(p, tri.p2, tri.p3);
            float d3 = sign(p, tri.p3, tri.p1);
            bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
            return !(has_neg && has_pos);
        };

        if (isPointInsideTriangle(t1, t2.p1) || isPointInsideTriangle(t1, t2.p2) || isPointInsideTriangle(t1, t2.p3))
            return true;
        if (isPointInsideTriangle(t2, t1.p1) || isPointInsideTriangle(t2, t1.p2) || isPointInsideTriangle(t2, t1.p3))
            return true;

        return true; // triangles overlap
    }

    bool Shape::collision(const Line& l1, const Line& l2) {
        auto cross = [](const Vector2& a, const Vector2& b) {
            return a.x * b.y - a.y * b.x;
        };

        Vector2 r = l1.end - l1.start;
        Vector2 s = l2.end - l2.start;
        Vector2 diff = l2.start - l1.start;

        float rxs = cross(r, s);
        float qpxr = cross(diff, r);

        if (std::abs(rxs) < 1e-6f) {
            // Lines are parallel
            if (std::abs(qpxr) < 1e-6f) {
                // Lines are collinear, check for overlap
                float t0 = ((l2.start - l1.start).dot(r)) / (r.dot(r));
                float t1 = ((l2.end   - l1.start).dot(r)) / (r.dot(r));
                if ((t0 >= 0 && t0 <= 1) || (t1 >= 0 && t1 <= 1) || 
                    (t0 < 0 && t1 > 1) || (t1 < 0 && t0 > 1)) {
                    return true; // overlapping segments
                }
                return false;
            }
            return false; // parallel but not collinear
        }

        float t = cross(diff, s) / rxs;
        float u = cross(diff, r) / rxs;

        return t >= 0 && t <= 1 && u >= 0 && u <= 1;
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

    bool Shape::collision(const Rectangle& rect, const Circle& circle) {
        float closestX = std::max(rect.x, std::min(circle.x, rect.x + rect.w));
        float closestY = std::max(rect.y, std::min(circle.y, rect.y + rect.h));

        float dx = circle.x - closestX;
        float dy = circle.y - closestY;

        return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
    }

    bool Shape::collision(const Quad& quad, const Circle& circle) {
        auto closestPointOnSegment = [](const Vector2& A, const Vector2& B, const Vector2& P) -> Vector2 {
            Vector2 AB = B - A;
            float ab2 = AB.x * AB.x + AB.y * AB.y;
            if (ab2 < 1e-6f) return A; // A and B are the same point
            Vector2 AP = P - A;
            float t = (AP.x * AB.x + AP.y * AB.y) / ab2;
            t = fmax(0.0f, fmin(1.0f, t));
            return A + AB * t;
        };

        Vector2 C{circle.x, circle.y};
        Vector2 v[4] = { quad.p1, quad.p2, quad.p3, quad.p4 };

        if (isPointInside(quad, C)) {
            return true;
        }

        for (int i = 0; i < 4; ++i) {
            Vector2 a = v[i];
            Vector2 b = v[(i + 1) % 4];
            Vector2 q = closestPointOnSegment(a, b, C);
            if (distanceBetween(q, C) <= circle.radius) {
                return true;
            }
        }

        for (int i = 0; i < 4; ++i) {
            if (distanceBetween(v[i], C) <= circle.radius) {
                return true;
            }
        }

        return false;
    }

    bool Shape::collision(const Quad& q, const Triangle& t) {
        auto project = [](const Vector2* vertices, int num_vertices, const Vector2& axis) {
            float min = vertices[0].dot(axis);
            float max = min;
            for (int i = 1; i < num_vertices; i++) {
                float p = vertices[i].dot(axis);
                if (p < min) min = p;
                else if (p > max) max = p;
            }
            return std::make_pair(min, max);
        };

        auto getAxes = [](const Vector2* vertices, int num_vertices) {
            std::vector<Vector2> axes;
            for (int i = 0; i < num_vertices; i++) {
                Vector2 edge = vertices[(i + 1) % num_vertices] - vertices[i];
                Vector2 axis(-edge.y, edge.x);  // Perpendicular
                float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
                if (len > 1e-6f) {
                    axis.x /= len;
                    axis.y /= len;
                    axes.push_back(axis);
                }
            }
            return axes;
        };

        Vector2 q_vertices[] = { q.p1, q.p2, q.p3, q.p4 };
        Vector2 t_vertices[] = { t.p1, t.p2, t.p3 };

        std::vector<Vector2> axes = getAxes(q_vertices, 4);
        std::vector<Vector2> triAxes = getAxes(t_vertices, 3);
        axes.insert(axes.end(), triAxes.begin(), triAxes.end());

        const float EPS = 1e-6f;
        for (const auto& axis : axes) {
            auto q_proj = project(q_vertices, 4, axis);
            auto t_proj = project(t_vertices, 3, axis);
            if (q_proj.second < t_proj.first - EPS || t_proj.second < q_proj.first - EPS) {
                return false;  // Found separating axis
            }
        }

        // Check if any vertex of one shape is inside the other
        for (const auto& v : t_vertices) if (isPointInside(q, v)) return true;
        for (const auto& v : q_vertices) if (isPointInside(t, v)) return true;

        return true;  // No separating axis, collision confirmed
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
        else {
            fatal_error("Error: Invalid Vector2 assignment.");
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