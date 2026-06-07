//  #include "plugin_path/plugin.cpp"
// plugin_includes

namespace Amara {
    class NodeFactory;

    class Plugins {
    public:
        static NodeFactory* nodeFactory;

        template <typename T>
        static void registerNode(std::string);

        static void bind_lua(sol::state& lua) {
            // NodeName::bind_lua(lua);
            // plugin_lua_bindings
        }

        static void registerNodes() {
            // registerNode<NodeName>("NodeName");
            // plugin_node_registrations
        }
    };
}