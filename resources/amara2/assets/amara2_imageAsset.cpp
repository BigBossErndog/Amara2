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

        ImageAsset(Amara::GameProps* _gameProps): Amara::Asset(_gameProps) {
            type = AssetEnum::Image;
            typeKey = "ImageAsset";
        }
        
        bool loadImage(std::string _p) {
            path = gameProps->system->getAssetPath(_p);
            
            if (!gameProps->system->fileExists(path)) {
                debug_log("Error: Image file not found \"", path, "\".");
                return false;
            }
            
            clearTexture();

            SDL_IOStream *rw = SDL_IOFromFile(path.c_str(), "rb");
            if (!rw) {
                debug_log("Error: Failed to open file: ", SDL_GetError());
                return false;
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            unsigned char *buffer = (unsigned char*)SDL_malloc(fileSize);
            SDL_ReadIO(rw, buffer, fileSize);
            SDL_CloseIO(rw);

            if (Amara::Encryption::is_buffer_encrypted(buffer, fileSize)) {
                #if defined(AMARA_ENCRYPTION_KEY)
                    Amara::Encryption::decryptBuffer(buffer, fileSize, AMARA_ENCRYPTION_KEY)
                #else
                    debug_log("Error: Attempted to load encrypted data without encryption key. \"", path, "\".");
                    SDL_free(buffer);
                    gameProps->breakWorld();
                    return false;
                #endif
            }

            stbi_set_flip_vertically_on_load(0);
            unsigned char *imageData = stbi_load_from_memory(buffer, fileSize, &width, &height, &channels, 4);
            SDL_free(buffer);

            if (!imageData) {
                debug_log("Error: Failed to load image data: ", path);
                return false;
            }
            
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                texture = SDL_CreateTexture(gameProps->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
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
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGenTextures(1, &glTextureID);
                
                if (glTextureID == 0) {
                    debug_log("Error: Texture generation failed. ", path);
                    stbi_image_free(imageData);
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
                stbi_image_free(imageData);
                
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
            #ifdef AMARA_OPENGL
            if (glTextureID != 0) {
                gameProps->queue_texture_garbage(glTextureID);
                glTextureID = 0;
            }
            #endif
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
            // TODO: Create a single pixel.
        }
    };
}