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
        float s = sin(rotation);
        float c = cos(rotation);

        float dx = v2.x - v1.x;
        float dy = v2.y - v1.y;

        float newX = dx * c - dy * s;
        float newY = dx * s + dy * c;
        
        return { v1.x + newX, v1.y + newY };
    }

    Vector2 centerOf(const Rectangle& rect) {
        return {
            static_cast<float>(rect.x + rect.w/2.0),
            static_cast<float>(rect.y + rect.h/2.0)
        };
    }
}