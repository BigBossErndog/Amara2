namespace Amara {
    class ImageAsset: public Amara::Asset {
    public:
        int width = 0;
        int height = 0;
        int channels = 0;
        int pitch = 0;

        SDL_Texture* texture = nullptr;
        SDL_GPUTexture* gpuTexture = nullptr;
        #ifdef AMARA_OPENGL
        GLuint glTextureID = 0;
        #endif

        unsigned char* imageData = nullptr;

        ImageAsset(Amara::GameProps* _gameProps): Amara::Asset(_gameProps) {
            type = AssetEnum::Image;
            typeKey = "ImageAsset";
        }

        ImageAsset(
            Amara::GameProps* _gameProps,
            unsigned char* _imageData,
            int _width,
            int _height,
            int _channels,
            int _pitch
        ): ImageAsset(_gameProps) {
            imageData = _imageData;
            width = _width;
            height = _height;
            channels = _channels;
            pitch = _pitch;
        }
        
        bool loadImage(std::string _p) {
            clearTexture();
            path = gameProps->system->getAssetPath(_p);

            std::string contents = gameProps->system->readFile(path);
            
            stbi_set_flip_vertically_on_load(0);
            imageData = stbi_load_from_memory((const unsigned char*)contents.data(), contents.size(), &width, &height, &channels, 4);

            if (!imageData) {
                fatal_error("Error: Failed to load image data: ", path);
                return false;
            }
            
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                texture = SDL_CreateTexture(gameProps->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
                if (!texture) {
                    fatal_error("Error: Failed to create texture: ", SDL_GetError());
                    stbi_image_free(imageData);
                    imageData = nullptr;
                    return false;
                }
                pitch = width * 4;
                SDL_UpdateTexture(texture, NULL, imageData, pitch);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                return true;
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGenTextures(1, &glTextureID);
                
                if (glTextureID == 0) {
                    fatal_error("Error: Texture generation failed. ", path);
                    stbi_image_free(imageData);
                    imageData = nullptr;
                    return false;
                }

                glBindTexture(GL_TEXTURE_2D, glTextureID);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                glGenerateMipmap(GL_TEXTURE_2D);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                glBindTexture(GL_TEXTURE_2D, 0);
                
                return true;
            }
            #endif
            else if (gameProps->gpuDevice) {
                SDL_GPUTextureCreateInfo textureInfo = {
                    .type = SDL_GPU_TEXTURETYPE_2D,
                    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                    .width = (Uint32)width,
                    .height = (Uint32)height,
                    .layer_count_or_depth = 1,
                    .num_levels = 1,
                    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
                };
                gpuTexture = SDL_CreateGPUTexture(gameProps->gpuDevice, &textureInfo);

                SDL_SetGPUTextureName(gameProps->gpuDevice, gpuTexture, key.c_str());

                SDL_GPUSamplerCreateInfo samplerInfo = {
                    .min_filter = SDL_GPU_FILTER_NEAREST,
                    .mag_filter = SDL_GPU_FILTER_NEAREST,
                    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
                };
                SDL_GPUSampler* sampler = SDL_CreateGPUSampler(gameProps->gpuDevice, &samplerInfo);

                SDL_GPUTransferBufferCreateInfo bufferInfo = {
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = (sizeof(PositionTextureVertex) * 4) + (sizeof(Uint16) * 6)
                };
                SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
                    gameProps->gpuDevice,
                    &bufferInfo
                );
            
                PositionTextureVertex* transferData = static_cast<PositionTextureVertex*>(SDL_MapGPUTransferBuffer(
                    gameProps->gpuDevice,
                    bufferTransferBuffer,
                    false
                ));
            
                transferData[0] = (PositionTextureVertex) { -1,  1, 0, 0, 0 };
                transferData[1] = (PositionTextureVertex) {  1,  1, 0, 4, 0 };
                transferData[2] = (PositionTextureVertex) {  1, -1, 0, 4, 4 };
                transferData[3] = (PositionTextureVertex) { -1, -1, 0, 0, 4 };

                Uint16* indexData = (Uint16*) &transferData[4];
                indexData[0] = 0;
                indexData[1] = 1;
                indexData[2] = 2;
                indexData[3] = 0;
                indexData[4] = 2;
                indexData[5] = 3;

                SDL_UnmapGPUTransferBuffer(gameProps->gpuDevice, bufferTransferBuffer);

                SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = static_cast<Uint32>(width * height * 4)
                }; 
                SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
                    gameProps->gpuDevice,
                    &transferBufferInfo
                );
                
                Uint8* textureTransferPtr = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(
                    gameProps->gpuDevice,
                    textureTransferBuffer,
                    false
                ));
                SDL_memcpy(textureTransferPtr, imageData, width * height * 4);
                SDL_UnmapGPUTransferBuffer(gameProps->gpuDevice, textureTransferBuffer);

                SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gameProps->gpuDevice);
	            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
                return true;
            }
            fatal_error("Error: Failed to load image, no graphics available. \"", path, "\"");
            return false;
        }

        unsigned char* resize(int _w, int _h, bool keep) {
            if (!imageData) {
                fatal_error("Error: ImageAsset contains no image data.");
                return nullptr;
            }
            unsigned char* result_data = new unsigned char[_w * _h * channels];

            stbir_resize_uint8_linear(
                imageData, width, height, 0,
                result_data, _w, _h, 0,
                (stbir_pixel_layout)channels
            );

            if (keep) {
                stbi_image_free(imageData);
                imageData = result_data;
                width = _w;
                height = _h;
            }
            
            return result_data;
        }
        unsigned char* resize(int _w, int _h) {
            return resize(_w, _h, false);
        }

        virtual void clearTexture() {
            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }
            #ifdef AMARA_OPENGL
            if (glTextureID != 0) {
                gameProps->queue_texture_garbage(glTextureID);
                glTextureID = 0;
            }
            #endif
            if (imageData) {
                stbi_image_free(imageData);
                imageData = nullptr;
            }
        }

        virtual void destroy() override {
            clearTexture();
            Amara::Asset::destroy();
        }
    };

    class SpritesheetAsset: public Amara::ImageAsset {
    public:
        SpritesheetAsset(Amara::GameProps* _gameProps): ImageAsset(_gameProps) {
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

    class SinglePixelAsset: public Amara::ImageAsset {
    public:
        SinglePixelAsset(Amara::GameProps* _gameProps): ImageAsset(_gameProps) {
            typeKey = "SinglePixelAsset";
            
            createPixel();
        }

        void createPixel() {
            clearTexture();

            width = 1;
            height = 1;
            channels = 4; // RGBA
            pitch = width * 4;

            unsigned char pixelData[] = {255, 255, 255, 255}; // White, Opaque (R, G, B, A)

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                texture = SDL_CreateTexture(gameProps->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
                if (!texture) {
                    fatal_error("Error: Failed to create 1x1 texture for SinglePixelAsset: ", SDL_GetError());
                    return;
                }
                SDL_UpdateTexture(texture, NULL, pixelData, pitch);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGenTextures(1, &glTextureID);
                
                if (glTextureID == 0) {
                    fatal_error("Error: OpenGL texture generation failed for SinglePixelAsset.");
                    return;
                }

                glBindTexture(GL_TEXTURE_2D, glTextureID);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
                // Mipmaps are not strictly necessary for a 1x1 texture with nearest filtering, but kept for consistency.
                glGenerateMipmap(GL_TEXTURE_2D); 
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            #endif
            else if (gameProps->gpuDevice) {
                // TODO: SDL_GPU signle pixel
            }
        }
    };
}