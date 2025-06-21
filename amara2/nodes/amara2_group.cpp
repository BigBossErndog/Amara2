namespace Amara {
    class Group: public Amara::Node {
    public:
        Group() {
            set_base_node_id("Group");
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Group>("Group",
                sol::base_classes, sol::bases<Amara::Node>()
            );
        }
    };
}