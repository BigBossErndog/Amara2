namespace Amara {
    class Button {
    public:
        Button() {}

        bool isDown = false;
        bool justPressed = false;
        bool justReleased = false;

        double timeHeld = 0;

        void press() {
            if (!isDown) {
                isDown = true;
                timeHeld = 0;
                justPressed = true;
            }
        }

        void release() {
            if (isDown) {
                isDown = false;
                justReleased = true;
            }
        }

        void manage(double deltaTime) {
            justPressed = false;
            justReleased = false;
            if (isDown) {
                timeHeld += deltaTime;
            }
        },

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::Button>("")
        }
    };
}