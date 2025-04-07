namespace Amara {
    struct PositionColorVertex {
        float x, y, z;
        Uint8 r, g, b, a;
    };

    typedef struct PositionTextureVertex {
        float x, y, z;
        float u, v;
    };
}