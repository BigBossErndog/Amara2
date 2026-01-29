namespace Amara {
    class ResolutionLayer: public Amara::TextureContainer {
    public:
        ResolutionLayer(): Amara::TextureContainer() {
            set_base_node_id("ResolutionLayer");
        }
    
        virtual void drawObjects(const Rectangle& v) override {
            pos = Vector2(0, 0);
            rotation = 0;
            
            scale = Vector2(
                v.w / width,
                v.h / height
            );
    
            TextureContainer::drawObjects(v);
        }
    
        virtual void drawChildren(const Rectangle& v) override {
            PassOnProps rec_props = gameProps->passOn;
    
            passOn.insideTextureContainer = true;
            gameProps->passOn = passOn;
            gameProps->passOn.scale /= scale;
    
            Amara::Node::drawChildren(v);
            
            gameProps->passOn = rec_props;
            passOn = rec_props;
        }
    
        virtual void pass_on_properties() override {
            if (fixedToCamera && !gameProps->passOn.texturePropsLock) {
                gameProps->passOn.reset(true);
            }
            passOn = gameProps->passOn;
            
            if (passOnPropsEnabled) {
                gameProps->passOn = passOn;
            }
        }
    
        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ResolutionLayer>("ResolutionLayer",
                sol::base_classes, sol::bases<Amara::TextureContainer, Amara::Node>()
            );
        }
    };
}