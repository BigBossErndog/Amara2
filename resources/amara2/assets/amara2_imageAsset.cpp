namespace Amara {
    class ImageAsset: public Asset {
    public:
        ImageAsset(): Asset() {
            type = AssetEnum::Image;
        }

        int width = 0;
        int height = 0;
        int channels = 0;
        int pitch = 0;

        SDL_Texture* texture = nullptr;
        GLuint glTextureID = 0;

        bool loadImage(std::string path) {
            unsigned char *imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);
            if (!imageData) {
                debug_log("Error: Failed to load image: ", path);
                return false;
            }

            if (Props::glContext != NULL) {
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(imageData);  // Free image memory
                
                return true;
            }
            else if (Props::renderer) {
                texture = SDL_CreateTexture(Props::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
                if (!texture) {
                    debug_log("Error: Failed to create texture: ", SDL_GetError());
                    stbi_image_free(imageData);
                    return false;
                }

                pitch = width * channels;
                SDL_UpdateTexture(texture, NULL, imageData, pitch);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                stbi_image_free(imageData);

                return true;
            }
            return false;
        }
    };

    class SpritesheetAsset: public ImageAsset {
    public:
        SpritesheetAsset(): ImageAsset() {
            type = AssetEnum::Spritesheet;
        }

        float frameWidth = 0;
        float frameHeight = 0;

        bool loadSpritesheet(std::string path, int _fw, int _fh) {
            frameWidth = _fw;
            frameHeight = _fh;

            return ImageAsset::loadImage(path);
        }
    };
}