namespace Amara {
    class ShaderCamera_Container: public Amara::ShaderLayer {
    public:
        Amara::Camera* camera = nullptr;

        ShaderCamera_Container() : Amara::ShaderLayer() {}

        virtual void drawChildren(const Rectangle& v) override {
            PassOnProps rec_props = gameProps->passOn;

            passOn.insideTextureContainer = true;
            gameProps->passOn = passOn;

            children_copy_list = camera->parent->children;

            Amara::Node* child;
			for (auto it = children_copy_list.begin(); it != children_copy_list.end();) {
                child = *it;
				if (child == nullptr || child->destroyed || !child->visible || child->parent != camera->parent) {
					++it;
					continue;
				}

                if (passOn.insideCamera && child->is_camera) {
                    ++it;
                    continue;
                }
                
                update_properties();
				child->draw(v);
                
                gameProps->passOn = passOn;
				++it;
			}

            gameProps->passOn = rec_props;
            passOn = rec_props;
        }
    };

    class ShaderCamera: public Amara::Camera {
    public:
        ShaderCamera_Container* cont = nullptr;

        ShaderCamera() {
            set_base_node_id("ShaderCamera");
            is_camera = true;
        }

        void init() override {
            Camera::init();

            if (!cont) {
                cont = new ShaderCamera_Container();
                cont->gameProps = gameProps;
                cont->camera = this;
                cont->init();

                addChild(cont);
            }
        }

        Amara::Node* configure(nlohmann::json config) override {
            if (config.find("shaderPasses") != config.end()) {
                if (config["shaderPasses"].is_array()) {
                    for (nlohmann::json s: config["shaderPasses"]) {
                        cont->addShaderPass(s);
                    }
                }
                else if (config["shaderPasses"].is_string()) {
                    cont->addShaderPass(config["shaderPasses"]);
                }
            }
            if (config.find("shaderPass") != config.end()) {
                if (config["shaderPass"].is_string()) {
                    cont->addShaderPass(config["shaderPass"]);
                }
            }
            if (config.find("shaderUniforms") != config.end()) {
                nlohmann::json uniforms = config["shaderUniforms"];
                cont->configure({
                    { "shaderUniforms", uniforms }
                });
            }
            if (config.find("repeats") != config.end()) {
                cont->repeats = config["repeats"];
            }
            if (config.find("tint") != config.end()) {
                cont->tint = config["tint"];
            }
            if (config.find("fill") != config.end()) {
                cont->fill = config["fill"];
            }
            if (config.find("blendMode") != config.end()) {
                cont->blendMode = static_cast<Amara::BlendMode>(config["blendMode"].get<int>());
            }

            return Amara::Camera::configure(config);
        }

        void drawChildren(const Rectangle& v) override {
            if (parent->children.size() == 0) return;
            
            children_copy_list = parent->children;

            Vector2 vcenter = Vector2(v.w/2.0f, v.h/2.0f);

            if (sizeTethered) {
                viewport = v;
                width = ((v.w / passOn.scale.x) / passOn.zoom.x)/passOn.window_zoom.x;
                height = ((v.h / passOn.scale.y) / passOn.zoom.y)/passOn.window_zoom.y;
            }
            else {
                viewport = Rectangle(
                    vcenter.x + (pos.x - (width*origin.x)*scale.x*passOn.scale.x)*passOn.zoom.x*passOn.window_zoom.x,
                    vcenter.y + (pos.y - (height*origin.y)*scale.y*passOn.scale.y - pos.z)*passOn.zoom.y*passOn.window_zoom.y,
                    width*scale.x*passOn.scale.x*passOn.zoom.x*passOn.window_zoom.x,
                    height*scale.y*passOn.scale.x*passOn.zoom.y*passOn.window_zoom.y
                );
            }

            SDL_Rect old_sdl_viewport;
            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_GetRenderViewport(gameProps->renderer, &old_sdl_viewport);
                
                SDL_Rect setv = Rectangle::makeSDLRect(viewport);
                SDL_SetRenderViewport(gameProps->renderer, &setv);
            }
            #ifdef AMARA_OPENGL
            GLint old_gl_viewport[4];
            if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                glGetIntegerv(GL_VIEWPORT, old_gl_viewport);
                glViewport(viewport.x, gameProps->master_viewport.h - (viewport.y + viewport.h), viewport.w, viewport.h);
            }
            #endif
            
            PassOnProps rec_passOn = gameProps->passOn;
            pass_on_properties();

            passOn.insideCamera = true;
            gameProps->passOn = passOn;

            if (cont) {
                cont->draw(viewport);
            }
            gameProps->passOn = rec_passOn;

            if (gameProps->graphics == GraphicsEnum::Render2D && gameProps->renderer) {
                SDL_SetRenderViewport(gameProps->renderer, &old_sdl_viewport);
            }
            #ifdef AMARA_OPENGL
            else if (gameProps->graphics == GraphicsEnum::OpenGL && gameProps->glContext != NULL) {
                gameProps->renderBatch->flush();
                glViewport(old_gl_viewport[0], old_gl_viewport[1], old_gl_viewport[2], old_gl_viewport[3]);
            }
            #endif
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ShaderCamera>("ShaderCamera",
                sol::base_classes, sol::bases<Amara::Camera>(),
                "repeats", sol::property(
                    [](Amara::ShaderCamera& sc) -> int {
                        if (sc.cont) {
                            return sc.cont->repeats;
                        }
                        return 0;
                    },
                    [](Amara::ShaderCamera& sc, int val) {
                        if (sc.cont) {
                            sc.cont->repeats = val;
                        }
                    }
                ),
                "shaderPasses", sol::property(
                    [](Amara::ShaderCamera& sc) -> sol::table {
                        if (sc.cont) {
                            if (sc.cont->shader_passes.size() == 0) return sol::nil;

                            sol::state& lua = sc.gameProps->lua;
                            sol::table tbl = lua.create_table();
                            int index = 1;
                            for (Amara::ShaderProgram* prog: sc.cont->shader_passes) {
                                tbl[index] = prog;
                                index += 1;
                            }
                            return tbl;
                        }
                        return sol::table();
                    },
                    [](Amara::ShaderCamera& sc, sol::object val) {
                        if (sc.cont) {
                            sc.cont->luaAddShaderPass(val);
                        }
                    }
                ),
                "shaderPass", sol::property(
                    [](Amara::ShaderCamera& sc) -> sol::object {
                        if (sc.cont) {
                            if (sc.cont->shader_passes.size() == 0) return sol::nil;
                            if (sc.cont->shader_passes.size() == 1) return sol::make_object(sc.gameProps->lua, sc.cont->shader_passes[0]->key);

                            sol::state_view lua = sc.gameProps->lua;
                            sol::table tbl = lua.create_table();
                            int index = 1;
                            for (Amara::ShaderProgram* prog: sc.cont->shader_passes) {
                                tbl[index] = prog;
                                index += 1;
                            }
                            return tbl;
                        }
                        return sol::table();
                    },
                    [](Amara::ShaderCamera& sc, sol::object val) {
                        if (sc.cont) {
                            sc.cont->luaAddShaderPass(val);
                        }
                    }
                )
            );
        }
    };
}