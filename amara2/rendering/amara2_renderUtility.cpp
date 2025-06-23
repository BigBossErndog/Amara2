namespace Amara {
    #if defined(_WIN32)
    std::vector<GraphicsEnum> Amara_Default_Graphics_Priority = {
        #ifdef AMARA_OPENGL
        GraphicsEnum::OpenGL,
        #endif
        GraphicsEnum::Render2D,
        GraphicsEnum::VulkanMetalDirectX
    };
    #else
    std::vector<GraphicsEnum> Amara_Default_Graphics_Priority = {
        #ifdef AMARA_OPENGL
        GraphicsEnum::OpenGL,
        #endif
        GraphicsEnum::Render2D,
        GraphicsEnum::VulkanMetalDirectX,
    };
    #endif

    struct PositionColorVertex {
        float x, y, z;
        Uint8 r, g, b, a;
    };

    typedef struct PositionTextureVertex {
        float x, y, z;
        float u, v;
    };

    SDL_BlendMode AMARA_BLENDMODE_MASK = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO,
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ZERO,
        SDL_BLENDFACTOR_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD
    );
    
    SDL_BlendMode AMARA_BLENDMODE_ERASER = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO,
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ZERO,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD
    );

    void Apply_SDL_BlendMode(Amara::GameProps* gameProps, SDL_Texture* texture, Amara::BlendMode blendMode) {
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
            case BlendMode::Mask:
                if (!gameProps->passOn.insideTextureContainer) {
                    debug_log("Error: Mask and Erase blend modes can only be used inside a TextureContainer or TextureCamera.");
                    gameProps->breakWorld();
                }
                SDL_SetTextureBlendMode(texture, AMARA_BLENDMODE_MASK);
                break;
            case BlendMode::Erase:
                if (!gameProps->passOn.insideTextureContainer) {
                    debug_log("Error: Mask and Erase blend modes can only be used inside a TextureContainer or TextureCamera.");
                    gameProps->breakWorld();
                }
                SDL_SetTextureBlendMode(texture, AMARA_BLENDMODE_ERASER);
                break;
            default:
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
                break;
        }
    }
}