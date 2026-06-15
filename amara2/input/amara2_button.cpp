namespace Amara {
    class Button {
    public:
        Button() {}

        bool isDown = false;
        bool justPressed = false;
        bool justReleased = false;

        double timeHeld = 0;
        double timeSinceHeld = 0;

        void reset() {
            isDown = false;
            justPressed = false;
            justReleased = false;
            timeHeld = 0;
            timeSinceHeld = 0;
        }
        
        bool press() {
            if (!isDown) {
                isDown = true;
                timeHeld = 0;
                timeSinceHeld = 0;
                justPressed = true;
                return true;
            }
            return false;
        }

        bool release() {
            if (isDown) {
                isDown = false;
                timeSinceHeld = 0;
                justReleased = true;
                return true;
            }
            return false;
        }

        void update(double deltaTime) {
            justPressed = false;
            justReleased = false;
            if (isDown) {
                timeHeld += deltaTime;
            }
            else {
                timeSinceHeld += deltaTime;
            }
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Button>("ButtonState",
                "isDown", sol::readonly(&Button::isDown),
                "justPressed", sol::readonly(&Button::justPressed),
                "justReleased", sol::readonly(&Button::justReleased),
                "timeHeld", sol::readonly(&Button::timeHeld),
                "timeSinceHeld", sol::readonly(&Button::timeSinceHeld)
            );
        }
    };
}