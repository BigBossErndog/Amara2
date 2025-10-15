namespace Amara {
    class TouchHandler {
    public:
        sol::object luaobject;
        
        std::vector<Pointer*> pointers;

        int count = 0;

        Pointer* lastFinger = nullptr;

        Pointer* getFinger(SDL_FingerID fingerID) {
            for (auto p: pointers) {
                if (p->id == fingerID) return p;
            }
            return nullptr;
        }

        Pointer* getNewFinger(SDL_FingerID fingerID) {
            Amara::Pointer* p;
            for (auto it = pointers.begin(); it != pointers.end(); ++it) {
                p = *it;
                if (!p->active) {
                    p->id = fingerID;
                    p->active = true;
                    p->state.press();
                    count++;
                    lastFinger = p;
                    return p;
                }
            }
            pointers.push_back(new Pointer());
            p = pointers.back();
            p->id = fingerID;
            p->active = true;
            lastFinger = p;
            count++;
            return p;
        }

        Amara::Pointer* activateAnyFinger(SDL_FingerID fingerID) {
            Pointer* p = getFinger(fingerID);
            if (p) return p;
            return getNewFinger(fingerID);
        }

        void deactivateFinger(SDL_FingerID fingerID) {
            Pointer* p;
            for (auto it = pointers.begin(); it != pointers.end(); ++it) {
                p = *it;
                if (p->id == fingerID) {
                    p->active = false;
                    p->state.release();
                    count--;
                    return;
                }
            }
        }

        void update(double deltaTime) {
            for (auto& p: pointers) {
                p->update(deltaTime);
            }
        }

        bool isDown() {
            for (auto& p: pointers) {
                if (p->active && p->state.isDown) return true;
            }
            return false;
        }
        bool justPressed() {
            for (auto& p: pointers) {
                if (p->active && p->state.justPressed) return true;
            }
            return false;
        }
        bool justReleased() {
            for (auto& p: pointers) {
                if (p->active && p->state.justReleased) return true;
            }
            return false;
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<TouchHandler>("TouchHandler",
                "count", sol::readonly(&TouchHandler::count),
                "isDown", sol::property(&TouchHandler::isDown),
                "justPressed", sol::property(&TouchHandler::justPressed),
                "justReleased", sol::property(&TouchHandler::justReleased)
            );
        }
    };
}