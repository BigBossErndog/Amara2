namespace Amara {
    template<typename... Args>
    void debug_log(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        std::cout << ss.str() << std::endl;
        
        #if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
        SDL_Log("%s", ss.str().c_str());
        #endif
    }

    template<typename... Args>
    void fatal_error(Args... args) {
        std::ostringstream ss;
        (ss << ... << args);
        throw std::runtime_error(ss.str());
    }

    template <class T> bool vector_contains(std::vector<T> list, T f) {
        for (T obj: list) 
            if (obj == f) return true;
        return false;
    }
    
    template <class T> void vector_append(std::vector<T>& list1, std::vector<T> list2) {
        for (T element: list2) list1.push_back(element);
    }

    float fixed_range(float num, float min, float max) {
        if (num < min) return min;
        if (num > max) return max;
        return num;
    }
    
    float abs_mod(float num, float den) {
        while (num < 0) num += den;
        float result = fmod(num, den);
        return result;
    }

    bool is_node(sol::object);

    std::string graphics_to_string(GraphicsEnum g) {
        switch (g) {
            case GraphicsEnum::Render2D: return "Render2D";
            case GraphicsEnum::OpenGL: return "OpenGL";
            case GraphicsEnum::Vulkan: return "Vulkan";
            case GraphicsEnum::DirectX: return "DirectX";
            case GraphicsEnum::DirectX_Legacy: return "DirectX_Legacy";
            case GraphicsEnum::VulkanMetalDirectX: return "GPURenderer";
            default: return "None";
        }
    }

    inline std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
        // TODO: Replace with a real Base64 decoding implementation ***
        debug_log("Warning: Base64 decoding is not implemented!");
        std::vector<unsigned char> decoded_data;
        return decoded_data;
    }

    // Zlib/Gzip Decompression
    inline std::vector<unsigned char> decompress_data(const std::vector<unsigned char>& compressed_data, const std::string& compression_type) {
        // TODO: Replace with real zlib/gzip decompression if needed ***
        if (compression_type == "zlib" || compression_type == "gzip") {
             debug_log("Warning: ", compression_type, " decompression is not implemented!");
        }
        return compressed_data;
    }
}