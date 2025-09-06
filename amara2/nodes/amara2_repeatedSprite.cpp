namespace Amara {
    class RepeatedSprite: public Amara::Sprite {
    public:
        double offsetX = 0;
        double offsetY = 0;

        RepeatedSprite(): Amara::Sprite() {
            set_base_node_id("RepeatedSprite");
        }

        virtual void drawTile(const Rectangle& v, int offsetX, int offsetY) override {
            if (image == nullptr) return;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<RepeatedSprite>("RepeatedSprite",
                sol::base_classes, sol::bases<Amara::Sprite, Amara::Node>(),
                "offsetX", &RepeatedSprite::offsetX,
                "offsetY", &RepeatedSprite::offsetY
            );
        }
    };
}