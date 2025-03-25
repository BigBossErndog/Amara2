namespace Amara {
    float toDegrees(float radians) {
        return radians*180/M_PI;
    }
    float toRadians(float degrees) {
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
        float startAngle = angleBetween(v1, v2);
        float dist = distanceBetween(v1, v2);
        float endAngle = startAngle + rotation;
        return {
            v1.x + sin(endAngle)*dist,
            v1.y + cos(endAngle)*dist
        };
    }

    Vector2 centerOf(const Rectangle& rect) {
        return {
            static_cast<float>(rect.x + rect.w/2.0),
            static_cast<float>(rect.y + rect.h/2.0)
        };
    }

    Vector2 getEdge(const Vector2& p1, const Vector2& p2) {
        return {p2.x - p1.x, p2.y - p1.y};
    }
    
    Vector2 getPerpendicular(const Vector2& edge) {
        return {-edge.y, edge.x};
    }
    
    float dotProduct(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }
    
    void projectQuad(const Quad& quad, const Vector2& axis, float& min, float& max) {
        Vector2 points[] = {quad.p1, quad.p2, quad.p3, quad.p4};
        min = max = dotProduct(points[0], axis);
        for (const auto& point : points) {
            float projection = dotProduct(point, axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }
    
    bool overlapOnAxis(const Quad& quad1, const Quad& quad2, const Vector2& axis) {
        float min1, max1, min2, max2;
        projectQuad(quad1, axis, min1, max1);
        projectQuad(quad2, axis, min2, max2);
        return !(max1 < min2 || max2 < min1);
    }
    
    bool checkQuadCollision(const Quad& quad1, const Quad& quad2) {
        Vector2 edges[] = {
            getPerpendicular(getEdge(quad1.p1, quad1.p2)),
            getPerpendicular(getEdge(quad1.p2, quad1.p3)),
            getPerpendicular(getEdge(quad1.p3, quad1.p4)),
            getPerpendicular(getEdge(quad1.p4, quad1.p1)),
            getPerpendicular(getEdge(quad2.p1, quad2.p2)),
            getPerpendicular(getEdge(quad2.p2, quad2.p3)),
            getPerpendicular(getEdge(quad2.p3, quad2.p4)),
            getPerpendicular(getEdge(quad2.p4, quad2.p1))
        };
        
        for (const auto& axis : edges) {
            if (!overlapOnAxis(quad1, quad2, axis)) {
                return false; // Separating axis found, no collision
            }
        }
        return true; // No separating axis found, collision detected
    }

    bool rectRotatedCollision(
        const Rectangle& rect1,
        float rotation1,
        const Vector2& origin1, 

        const Rectangle& rect2,
        float rotation2,
        const Vector2& origin2
    ) {
        Vector2 oPoint1 = { 
            rect1.x + rect1.w*origin1.x, 
            rect1.y + rect1.h*origin1.y
        };
        Quad quad1 = {
            rotateAroundAnchor(
                { rect1.x, rect1.y }, oPoint1, rotation1
            ),
            rotateAroundAnchor(
                { rect1.x + rect1.w, rect1.y }, oPoint1, rotation1
            ),
            rotateAroundAnchor(
                { rect1.x, rect1.y + rect1.h }, oPoint1, rotation1
            ),
            rotateAroundAnchor(
                { rect1.x + rect1.w, rect1.y + rect1.h }, oPoint1, rotation1
            )
        };

        Vector2 oPoint2 = { 
            rect2.x + rect2.w*origin2.x, 
            rect2.y + rect2.h*origin2.y
        };
        Quad quad2 = {
            rotateAroundAnchor(
                { rect2.x, rect2.y }, oPoint2, rotation2
            ),
            rotateAroundAnchor(
                { rect2.x + rect2.w, rect2.y }, oPoint2, rotation2
            ),
            rotateAroundAnchor(
                { rect2.x, rect2.y + rect2.h }, oPoint2, rotation2
            ),
            rotateAroundAnchor(
                { rect2.x + rect2.w, rect2.y + rect2.h }, oPoint2, rotation2
            )
        };

        return checkQuadCollision(quad1, quad2);
    }
}