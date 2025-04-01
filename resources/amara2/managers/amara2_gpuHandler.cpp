namespace Amara {
    class GPUHandler {
    public:
        GPUHandler() = default;
        
        SDL_GPUCommandBuffer* commandBuffer = nullptr;
        SDL_GPUTexture* swapChainTexture = nullptr;
        SDL_GPURenderPass* renderPass = nullptr;
    };
}