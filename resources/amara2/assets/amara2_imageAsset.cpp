namespace Amara {
    class ImageAsset: public Asset {
    public:
        ImageAsset(): Asset() {
            type = AssetEnum::Image;
            typeKey = "ImageAsset";
        }

        int width = 0;
        int height = 0;
        int channels = 0;
        int pitch = 0;

        SDL_Texture* texture = nullptr;
        GLuint glTextureID = 0;
        
        bool loadImage(std::string _p) {
            path = Props::files->getAssetPath(_p);

            if (!Props::files->fileExists(path)) {
                debug_log("Error: File not found at ", path);
                return false;
            }

            clearTexture();

            SDL_IOStream *rw = SDL_IOFromFile(path.c_str(), "rb");
            if (!rw) {
                debug_log("Failed to open file: ", SDL_GetError());
                return false;
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            unsigned char *buffer = (unsigned char*)SDL_malloc(fileSize);
            SDL_ReadIO(rw, buffer, fileSize);
            SDL_CloseIO(rw);

            stbi_set_flip_vertically_on_load(0);
            unsigned char *imageData = stbi_load_from_memory(buffer, fileSize, &width, &height, &channels, 4);
            SDL_free(buffer);

            if (!imageData) {
                debug_log("Error: Failed to load image data: ", path);
                return false;
            }

            if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
                GLuint textureID;
                glGenTextures(1, &textureID);
                
                if (texture == 0) {
                    debug_log("Error: Texture generation failed. ", path);
                    stbi_image_free(imageData);
                    return false;
                }

                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(imageData);  // Free image memory
                
                return true;
            }
            else if (Props::graphics == GraphicsEnum::Render2D && Props::renderer) {
                texture = SDL_CreateTexture(Props::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
                if (!texture) {
                    debug_log("Error: Failed to create texture: ", SDL_GetError());
                    stbi_image_free(imageData);
                    return false;
                }
                pitch = width * 4;
                SDL_UpdateTexture(texture, NULL, imageData, pitch);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                
                stbi_image_free(imageData);

                return true;
            }
            return false;
        }

        virtual void clearTexture() {
            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }
            if (glTextureID != 0) {
                Props::queue_texture_garbage(glTextureID);
                glTextureID = 0;
            }
        }

        virtual void destroy() override {
            clearTexture();
            Amara::Asset::destroy();
        }
    };

    class SpritesheetAsset: public ImageAsset {
    public:
        SpritesheetAsset(): ImageAsset() {
            type = AssetEnum::Spritesheet;
            typeKey = "SpritesheetAsset";
        }

        float frameWidth = 0;
        float frameHeight = 0;

        bool loadSpritesheet(std::string _p, int _fw, int _fh) {
            frameWidth = _fw;
            frameHeight = _fh;

            return ImageAsset::loadImage(_p);
        }
    };
}