namespace Amara {
    class ControlManager {
    public:
        Amara::GameProps* gameProps = nullptr;
        sol::object luaobject;
        sol::table props;
        
        std::unordered_map<std::string, ControlScheme*> controls;
        
        ControlManager() = default;
        
        void init(Amara::GameProps* _gameProps) {
            gameProps = _gameProps;
            
            if (!props.valid()) {
                props = gameProps->lua.create_table();
                sol::table props_meta = gameProps->lua.create_table();
                
                props_meta["__newindex"] = [this](sol::table tbl, sol::object key, sol::object config) {
                    if (!key.is<std::string>()) {
                        fatal_error("Error: ControlScheme expected a string key.");
                        return;
                    }
                    configureScheme(key.as<std::string>(), lua_to_json(config));
                };
                props_meta["__index"] = [this](sol::table tbl, sol::object key) -> sol::object {
                    if (!key.is<std::string>()) {
                        fatal_error("Error: ControlScheme expected a string key.");
                        return sol::nil;
                    }
                    if (controls.find(key.as<std::string>()) == controls.end()) return sol::nil;
                    return controls[key.as<std::string>()]->luaobject;
                };
                props[sol::metatable_key] = props_meta;
            }
        }
        
        nlohmann::json toData() {
            nlohmann::json data = nlohmann::json::object();
            for (auto& pair: controls) {
                data[pair.first] = pair.second->toData();
            }
            return data;
        }
        
        void configureScheme(std::string key, const nlohmann::json& config) {
            if (controls.find(key) == controls.end()) {
                createScheme(key);
                configureScheme(key, config);
                return;
            }
            ControlScheme* scheme = controls[key];
            
            if (config.is_null()) {
                scheme->clearAll();
                return;
            }
            
            if (json_has(config, "keys")) {
                nlohmann::json keys = config["keys"];
                if (keys.is_array()) {
                    scheme->clearKeys();
                    for (const auto& val: keys) {
                        scheme->addKey(static_cast<SDL_Keycode>((int)val));
                    }
                }
                else if (keys.is_number()) {
                    scheme->setKey(static_cast<SDL_Keycode>((int)keys));
                }
            }
            
            if (json_has(config, "buttons")) {
                nlohmann::json buttons = config["buttons"];
                if (buttons.is_array()) {
                    scheme->clearButtons();
                    for (const auto& val: buttons) {
                        scheme->addButton(static_cast<Amara::GamepadButton>((int)val));
                    }
                }
                else if (buttons.is_number()) {
                    scheme->setButton(static_cast<Amara::GamepadButton>((int)buttons));
                }
            }
        }
        
        ControlScheme* createScheme(std::string key) {
            controls[key] = new ControlScheme(key, gameProps);
            ControlScheme* scheme = controls[key];
            scheme->luaobject = sol::make_object(gameProps->lua, scheme);
            props[key] = scheme->luaobject;
            
            return scheme;
        }

        ControlScheme* getScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key];
            }
            return nullptr;
        }

        ControlScheme* scheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key];
            }
            return createScheme(key);
        }

        void removeScheme(std::string key) {
            if (controls.find(key) != controls.end()) {
                ControlScheme* scheme = controls[key];
                scheme->luaobject = sol::object(sol::nil);
                delete scheme;
                controls.erase(key);
            }
        }

        void clearAllSchemes() {
            for (const auto& pair: controls) {
                ControlScheme* scheme = pair.second;
                scheme->luaobject = sol::object(sol::nil);
                delete scheme;
            }
            controls.clear();
        }
        
        void clean_up() {
            clearAllSchemes();
            if (props.valid()) props = sol::nil;
        }

        bool isDown(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key]->isDown();
            }
            return false;
        }
        bool justPressed(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key]->justPressed();
            }
            return false;
        }
        bool justReleased(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key]->justReleased();
            }
            return false;
        }
        double timeHeld(std::string key) {
            if (controls.find(key) != controls.end()) {
                return controls[key]->timeHeld();
            }
            return false;
        }

        static void bind_lua(sol::state& lua) {
            Amara::Button::bind_lua(lua);
            Amara::Pointer::bind_lua(lua);
            Amara::Mouse::bind_lua(lua);
            Amara::TouchHandler::bind_lua(lua);
            Amara::KeyboardManager::bind_lua(lua);
            Amara::GamepadManager::bind_lua(lua);
            Amara::ControlScheme::bind_lua(lua);

            lua.new_usertype<ControlManager>("ControlManager",
                "props", &ControlManager::props,
                "get", &ControlManager::props,
                "config", sol::property(
                    [](ControlManager& cm, sol::object val) {
                        if (val.is<sol::table>()) {
                            nlohmann::json config = lua_to_json(val);
                            for (const auto& item: config.items()) {
                                cm.configureScheme(item.key(), item.value());
                            }
                        }
                    },
                    [](ControlManager& cm) {
                        return json_to_lua(cm.gameProps->lua, cm.toData());
                    }
                ),
                "createScheme", &ControlManager::createScheme,
                "getScheme", &ControlManager::getScheme,
                "scheme", &ControlManager::scheme,
                "removeScheme", &ControlManager::removeScheme,
                "clearAllSchemes", &ControlManager::clearAllSchemes,
                "isDown", &ControlManager::isDown,
                "justPressed", &ControlManager::justPressed,
                "justReleased", &ControlManager::justReleased,
                "timeHeld", &ControlManager::timeHeld,
                "mode", sol::property([](ControlManager& cm) {
                    return cm.gameProps->controlMode;
                })
            );
        }
    };
}