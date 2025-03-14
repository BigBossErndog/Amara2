namespace Amara {
    class Scene: public Amara::Entity {
    public:
        static void luaBind(sol::state& lua) {
            lua.new_usertype<Scene>("Scene",
                sol::base_classes, sol::bases<Amara::Entity>()
            );
        }
    };
}