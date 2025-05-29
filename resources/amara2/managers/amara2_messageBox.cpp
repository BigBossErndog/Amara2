namespace Amara {
    class MessageBox {
    public:
        Amara::GameProps* gameProps = nullptr;

        std::unordered_map<std::string, std::vector<sol::protected_function>> messageBox;

        bool active = false;

        MessageBox() = default;
    
        void send(std::string _key, sol::object _msgData) {
            gameProps->messages.send(this, _key, _msgData);
        }

        void listen(const std::string& key, sol::function action) {
            if (isListening(key)) {
                messageBox[key] = std::vector<sol::protected_function>();
            }
            messageBox[key].push_back(action);
        }

        bool isListening(const std::string& key) {
            return messageBox.find(key) != messageBox.end();
        }

        void activate() {
            active = true;
        }

        void deactivate() {
            active = false;
            for (Amara::Message& msg: gameProps->queue) {
                msg.forceRemove = true;
                msg.active = false;
            }
        }

        void run(bool _receiveMessages) {
            if (!messageBox.empty()) {
                MessageQueue* messages = gameProps->messages;

                for (
                    auto it = messages->begin();
                    it != messages->end();
                ) {
                    Message& msg = *it;
                    if (_receiveMessages && msg.active && messageBox.find(msg.key) != messageBox.end()) {
                        std::vector<sol::function>& list = messageBox[msg.key];
                        for (sol::function& callback: list) { 
                            callback(*this, gameProps->lua, msg.data);
                        }
                    }
                    if (msg.sender == this) {
                        it = messages->queue.erase(it);
                        continue;
                    }
                    ++it;
                }
                messageBox.clear();
            }
        }
        void run() {
            run(true);
        }

        void destroy() {
            deactivate();
        }
    };

    static void bind_lua(sol::state& lua) {
        lua.new_usertype<Amara::MessageBox>("MessageBox",
            "listen", &Amara::MessageBox::listen,
            "active", sol::readonly(&Amara::MessageBox::active),
            "send", &Amara::MessageBox::send,
            "listen", &Amara::MessageBox::listen,
            "isListening", &Amara::MessageBox::isListening,
            "activate", &Amara::MessageBox::activate,
            "deactivate", &Amara::MessageBox::deactivate
        );
    }
}