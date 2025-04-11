namespace Amara {
    class Asset {
    public:
        Asset() = default;

        AssetEnum type = AssetEnum::None;
        std::string typeKey = "Base Asset";
        std::string key;
        std::string path;

        bool destroyed = false;

        explicit operator std::string() const {
            return string_concat(
                "(", typeKey, ": \"",
                key, "\")"
            );
        }
        friend std::ostream& operator<<(std::ostream& os, const Asset& e) {
            return os << static_cast<std::string>(e);
        }
        
        template <typename T>
        T as();

        virtual void destroy() {
            destroyed = true;
        }

        virtual ~Asset() {}
    };
}