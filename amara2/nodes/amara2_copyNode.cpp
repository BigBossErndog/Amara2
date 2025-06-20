namespace Amara {
    class CopyNode: public Amara::Node {
    public:
        Amara::Node* copyTarget = nullptr;

        static std::vector<Amara::CopyNode*> copy_node_stack;
        
        CopyNode(): Amara::Node() {
            set_base_node_id("CopyNode");
        }

        virtual sol::object luaConfigure(std::string key, sol::object val) override {
            if (val.is<Amara::Node*>()) {
                if (String::equal(key, "target")) copy(val.as<Amara::Node*>());
            }
            return Amara::Node::luaConfigure(key, val);
        }

        sol::object copy(Amara::Node* _node) {
            if (_node == nullptr || _node->destroyed) return get_lua_object();
            copyTarget = _node;
            return get_lua_object();
        }

        virtual void run(double deltaTime) override {
            Amara::Node::run(deltaTime);
            if (copyTarget && copyTarget->destroyed) {
                copyTarget = nullptr;
            }
        }

        virtual void drawChildren(const Rectangle& v) override {
            pass_on_properties();

            for (Amara::CopyNode* node : copy_node_stack) {
                if (node == this) return;
            }
            copy_node_stack.push_back(this);

            if (copyTarget && copyTarget->destroyed) {
                copyTarget = nullptr;
            }
            if (copyTarget != nullptr) {
                update_properties();
                copyTarget->draw(v);
                gameProps->passOn = passOn;
            }

            children_copy_list = children;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != this) {
					++it;
					continue;
				}

                update_properties();
				child->draw(v);

                gameProps->passOn = passOn;
				++it;
			}

            for (auto it = copy_node_stack.begin(); it != copy_node_stack.end();) {
                if (*it == this) {
                    it = copy_node_stack.erase(it);
                    continue;
                }
                ++it;
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<CopyNode>("CopyNode",
                sol::base_classes, sol::bases<Amara::Node>(),
                "target", sol::property([](Amara::CopyNode& t) { return t.copyTarget; }, &Amara::CopyNode::copyTarget),
                "copy", &CopyNode::copy
            );
        }
    };

    std::vector<Amara::CopyNode*> Amara::CopyNode::copy_node_stack;
}