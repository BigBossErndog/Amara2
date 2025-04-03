namespace Amara {
    struct PositionColorVertex {
        float x, y, z;
        Uint8 r, g, b, a;
    };

    typedef struct PositionTextureVertex {
        float x, y, z;
        float u, v;
    };

    enum class BlendMode {
        None = 0,
        Alpha = 1,
        Additive = 2,
        Multiply = 3,
        PremultipliedAlpha = 4
    };
}