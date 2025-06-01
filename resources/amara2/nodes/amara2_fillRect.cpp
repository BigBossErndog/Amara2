namespace Amara {
    class FillRect: public Sprite {
    public:
        using Amara::Sprite::tint;

        float rectWidth = 1;
        float rectHeight = 1;

        FillRect(): Amara::Sprite() {
            set_base_node_id("FillRect");
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "color")) tint = config["color"];
            if (json_has(config, "width")) setWidth(json_extract(config, "width"));
            if (json_has(config, "height")) setHeight(json_extract(config, "height"));

            return Amara::Sprite::configure(config);
        }

        virtual void create() override {
            Amara::Sprite::create();
            image = gameProps->assets->whitePixel;
            textureWidth = image->width;
            textureHeight = image->height;

            setWidth(rectWidth);
            setHeight(rectHeight);
        }
        
        virtual bool setTexture(std::string key) override {
            bool success = Amara::Sprite::setTexture(key);
            if (success) {
                setWidth(rectWidth);
                setHeight(rectHeight);
            }
            return success;
        }

        float setWidth(float _w) {
            rectWidth = _w;
            return Amara::Sprite::setWidth(_w);
        }
        float setHeight(float _h) {
            rectHeight = _h;
            return Amara::Sprite::setHeight(_h);
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::FillRect>("FillRect",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "color", sol::property([](Amara::FillRect& t) -> Amara::Color { return t.tint; }, [](Amara::FillRect& t, sol::object c) { t.tint = c; }),
                "width", sol::property([](Amara::FillRect& t) -> float { return t.rectWidth; }, &Amara::FillRect::setWidth),
                "height", sol::property([](Amara::FillRect& t) -> float { return t.rectHeight; }, &Amara::FillRect::setHeight),
                "texture", sol::property([](Amara::FillRect& t) -> std::string { return t.image ? t.image->key : ""; }, &Amara::FillRect::setTexture)
            );
        }
    };
}