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

        static void bind_lua(sol::state& lua) {
            // Insert your lua bindings here
            
            /*
             * MyPlugin::bind_lua(lua);
            */
        }

        static void registerNodes() {
            // Insert your node registrations here.

            /*
             * e.g. registerNode<MyPlugin>("MyPlugin");
             */
        }

        static bool load(const LoadTask& task) {
            /* Insert loading your custom assets here.
             * Return true on success and 
             */
            switch (task.type) {

            }
        }
    };
}