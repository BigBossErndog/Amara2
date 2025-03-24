namespace Amara {
    /**
     * This is used for 2D sprites.
     */
    class Sprite: public Amara::Node {
    public:
        int frame = 0;

        ImageAsset* image = nullptr;
        SpritesheetAsset* spritesheet = nullptr;

        int imageWidth = 0;
        int imageHeight = 0;

        int frameWidth = 0;
        int frameHeight = 0;

        Vector2 origin = { 0.5, 0.5 };

        Sprite(): Amara::Node() {
            set_base_node_id("Sprite");
        }

        bool loadTexture(std::string key) {
            if (!Props::assets->has(key)) {
                debug_log("Error: Asset\"", key, "\" was not found.");
                return false;
            }
            image = Props::assets->get(key)->as<ImageAsset*>();
            if (image == nullptr) {
                debug_log("Error: Asset\"", key, "\" is not a valid texture asset.");
                return false;
            }
            imageWidth = image->width;
            imageHeight = image->height;

            spritesheet = image->as<SpritesheetAsset*>();
            if (spritesheet) {
                frameWidth = spritesheet->frameWidth;
                frameHeight = spritesheet->frameHeight;
            }
            else {
                frameWidth = 0;
                frameHeight = 0;
            }
        }

        virtual void update_properties() override {
            Props::passOn = passOn;
        }

        virtual void drawSelf(const Rectangle& v) override {
            
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Sprite>("Sprite",
                sol::base_classes, sol::bases<Node>(),
                "loadTexture", &Sprite::loadTexture,
                "imagew", sol::readonly(&Sprite::imageWidth),
                "imageh", sol::readonly(&Sprite::imageHeight),
                "framew", sol::readonly(&Sprite::frameWidth),
                "frameh", sol::readonly(&Sprite::frameHeight)
            );
        }
    };
}