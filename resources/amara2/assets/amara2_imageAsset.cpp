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
        SDL_GPUTexture* gpuTexture = nullptr;
        #ifdef AMARA_OPENGL
        GLuint glTextureID = 0;
        #endif
        
        bool loadImage(std::string _p) {
            path = Props::system->getAssetPath(_p);

            if (!Props::system->fileExists(path)) {
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
            
            if (Props::graphics == GraphicsEnum::Render2D && Props::renderer) {
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
            #ifdef AMARA_OPENGL
            else if (Props::graphics == GraphicsEnum::OpenGL && Props::glContext != NULL) {
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
            else if (Props::gpuDevice) {
                SDL_GPUTextureCreateInfo textureInfo = {
                    .type = SDL_GPU_TEXTURETYPE_2D,
                    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                    .width = (Uint32)width,
                    .height = (Uint32)height,
                    .layer_count_or_depth = 1,
                    .num_levels = 1,
                    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
                };
                gpuTexture = SDL_CreateGPUTexture(Props::gpuDevice, &textureInfo);

                SDL_SetGPUTextureName(Props::gpuDevice, gpuTexture, key.c_str());

                SDL_GPUSamplerCreateInfo samplerInfo = {
                    .min_filter = SDL_GPU_FILTER_NEAREST,
                    .mag_filter = SDL_GPU_FILTER_NEAREST,
                    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
                };
                SDL_GPUSampler* sampler = SDL_CreateGPUSampler(Props::gpuDevice, &samplerInfo);

                SDL_GPUTransferBufferCreateInfo bufferInfo = {
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = (sizeof(PositionTextureVertex) * 4) + (sizeof(Uint16) * 6)
                };
                SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
                    Props::gpuDevice,
                    &bufferInfo
                );
            
                PositionTextureVertex* transferData = static_cast<PositionTextureVertex*>(SDL_MapGPUTransferBuffer(
                    Props::gpuDevice,
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

                SDL_UnmapGPUTransferBuffer(Props::gpuDevice, bufferTransferBuffer);

                SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = static_cast<Uint32>(width * height * 4)
                }; 
                SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
                    Props::gpuDevice,
                    &transferBufferInfo
                );

                Uint8* textureTransferPtr = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(
                    Props::gpuDevice,
                    textureTransferBuffer,
                    false
                ));
                SDL_memcpy(textureTransferPtr, imageData, width * height * 4);
                SDL_UnmapGPUTransferBuffer(Props::gpuDevice, textureTransferBuffer);

                SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(Props::gpuDevice);
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
                Props::queue_texture_garbage(glTextureID);
                glTextureID = 0;
            }
            #endif
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