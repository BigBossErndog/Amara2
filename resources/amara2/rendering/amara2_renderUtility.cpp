namespace Amara {
    struct PositionColorVertex {
        float x, y, z;
        Uint8 r, g, b, a;
    };

    typedef struct PositionTextureVertex {
        float x, y, z;
        float u, v;
    };

    void setSDLBlendMode(SDL_Texture* texture, Amara::BlendMode blendMode) {
        switch (blendMode) {
            case BlendMode::Alpha:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                break;
            case BlendMode::Additive:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
                break;
            case BlendMode::Multiply:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MUL);
                break;
            case BlendMode::PremultipliedAlpha:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
                break; 
            default:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
                break;
        }
    }
}