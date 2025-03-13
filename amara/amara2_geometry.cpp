namespace Amara {
    typedef struct IntVector2 {
        int x = 0;
        int y = 0;
    } IntVector2;
    
    typedef struct FloatVector2 {
        float x = 0;
        float y = 0;
    } FloatVector2;

    class IntVector3: public IntVector2 {
        public: int z = 0;
    };

    class FloatVector3: public FloatVector2 {
        public: float z = 0;
    };

    typedef struct IntRect: public IntVector2 {
        int width = 0;
        int height = 0;
    } IntRect;

    typedef struct FloatRect: public FloatVector2 {
        float width = 0;
        float height = 0;
    } FloatRect;

    typedef struct FloatCircle: public FloatVector2 {
        float radius = 0;
    } FloatCircle;

    typedef struct FloatLine {
        FloatVector2 p1 = {0, 0};
        FloatVector2 p2 = {0, 0};
    } FloatLine;

    typedef struct FloatTriangle {
        FloatVector2 p1 = {0, 0};
        FloatVector2 p2 = {0, 0};
        FloatVector2 p3 = {0, 0};
    } FloatTriangle;
}