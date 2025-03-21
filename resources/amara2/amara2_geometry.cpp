namespace Amara {
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
}