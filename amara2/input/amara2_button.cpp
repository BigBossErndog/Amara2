namespace Amara {
    class Button {
    public:
        Button() {}

        bool isDown = false;
        bool justPressed = false;
        bool justReleased = false;

        double timeHeld = 0;
        double timeSinceHeld = 0;

        double pulseTimeRec = 0;

        void reset() {
            isDown = false;
            justPressed = false;
            justReleased = false;
            timeHeld = 0;
            timeSinceHeld = 0;
            pulseTimeRec = 0;
        }
        
        bool press() {
            if (!isDown) {
                isDown = true;
                timeHeld = 0;
                timeSinceHeld = 0;
                pulseTimeRec = 0;
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

        bool pulseHeld(double interim) {
            bool ret = justPressed;
            while (timeHeld - pulseTimeRec >= interim) {
                pulseTimeRec += interim;
                ret = true;
            }
            return ret;
        }
        bool pulseHeld(double interim, double delay) {
            return justPressed || (pulseHeld(interim) && timeHeld > delay);
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
                "timeSinceHeld", sol::readonly(&Button::timeSinceHeld),
                "pulseHeld", sol::overload(
                    sol::resolve<bool(double)>(&Button::pulseHeld),
                    sol::resolve<bool(double, double)>(&Button::pulseHeld)
                )
            );
        }
    };
}