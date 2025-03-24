namespace Amara {
    class Animation: public Amara::Action {
    public:
        Amara::Sprite* sprite = nullptr;

        Animation(): Amara::Action() {
            set_base_node_id("Animation");
        }

        virtual void prepare() override {
            if (sprite == nullptr) {
                sprite = actor->as<Amara::Sprite*>();
            }
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<Animation>("Animation"
                
            );
        }
    };
}