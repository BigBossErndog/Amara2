namespace Amara {
    class NodePool: public Amara::Node {
    public:
        NodePool(): Amara::Node() {
            set_base_node_id("NodePool");
        }

        virtual Amara::Node* addChild(Amara::Node* node) override {
            if (destroyed || node->parent == this) return node;

            node->deactivate();

            return Amara::Node::addChild(node);
        }

        Amara::Node* grab() {
            for (Amara::Node* child: children) {
                if (!child->isActive) {
                    child->activate();
                    return child;
                }
            }
            return nullptr;
        }

        sol::object lua_grab() {
            Amara::Node* grabbed = grab();
            if (grabbed) return grabbed->get_lua_object();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::NodePool>("NodePool"
                sol::base_classes, sol::bases<Amara::Node>(),
                "grab", &Amara::NodePool::lua_grab
            );
        }
    };
}