namespace Amara {
    class TouchHandler {
    public:
        std::vector<Pointer*> pointers;

        int count = 0;

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
                    count++;
                    return p;
                }
            }
            pointers.push_back(new Pointer());
            p = pointers.back();
            p->id = fingerID;
            p->active = true;
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

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<TouchHandler>("TouchHandler",
                "count", sol::readonly(&TouchHandler::count)
            );
        }
    };
}