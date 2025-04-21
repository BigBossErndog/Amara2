/*
 * Include your .cpp files here.
 */

namespace Amara {
    class NodeFactory;

    class Plugins {
    public:
        static NodeFactory* nodeFactory;

        template <typename T>
        static void registerNode(std::string);

        static void bindLua(sol::state& lua) {
            // Insert your lua bindings here
            
            /*
             * MyPlugin::bindLua(lua);
            */
        }

        static void registerNodes() {
            // Insert your node registrations here.

            /*
             * e.g. registerNode<MyPlugin>("MyPlugin");
             */
        }
    };
}