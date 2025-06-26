namespace Amara {
    class NodePool: public Amara::Group {
    public:
        NodePool(): Amara::Group() {
            set_base_node_id("NodePool");
        }

        virtual Amara::Node* addChild(Amara::Node* node) override {
            if (destroyed || node->parent == this) return node;

            node->deactivate();

            return Amara::Group::addChild(node);
        }

        Amara::Node* grab() {
            for (Amara::Node* child: children) {
                if (!child->isActive()) {
                    child->activate();

                    if (funcs.hasFunction("onGrab")) {
                        funcs.callFunction(this, "onGrab", child->get_lua_object());
                    }
                    return child;
                }
            }
            return nullptr;
        }

        sol::object lua_grab() {
            Amara::Node* grabbed = grab();
            if (grabbed) return grabbed->get_lua_object();
            return sol::nil;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::NodePool>("NodePool",
                sol::base_classes, sol::bases<Amara::Group, Amara::Node>(),
                "grab", &Amara::NodePool::lua_grab
            );
        }
    };
}