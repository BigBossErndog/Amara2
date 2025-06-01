namespace Amara {
    class Node;

    class MessageBox {
    public:
        Amara::GameProps* gameProps = nullptr;
        Amara::Node* node = nullptr;

        std::unordered_map<std::string, std::vector<sol::protected_function>> messageBox;

        bool active = false;

        MessageBox() = default;

        void init(Amara::GameProps* _gameProps, Amara::Node* _node) {
            gameProps = _gameProps;
            node = _node;
        }
    
        void send(std::string _key, sol::object _msgData) {
            gameProps->messages->send(this, _key, _msgData);
        }

        void listen(const std::string& key, sol::function action) {
            if (!isListening(key)) {
                messageBox[key] = std::vector<sol::protected_function>();
            }
            messageBox[key].push_back(action);
        }

        bool isListening(const std::string& key) {
            return messageBox.find(key) != messageBox.end();
        }

        void stopListening(const std::string& key) {
            if (isListening(key)) {
                messageBox.erase(key);
            }
        }
        void stopListening() {
            messageBox.clear();
        }

        void activate() {
            active = true;
        }

        void deactivate() {
            active = false;
            for (Amara::Message& msg: gameProps->messages->queue) {
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
                    if (_receiveMessages && msg.active) {
                        handleMessage(msg);
                    }
                    if (msg.sender == this) {
                        it = messages->queue.erase(it);
                        continue;
                    }
                    ++it;
                }
            }
        }
        void run() {
            run(true);
        }

        void handleMessage(const Message& msg);

        void destroy() {
            deactivate();
            messageBox.clear();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::MessageBox>("MessageBox",
                "active", sol::readonly(&Amara::MessageBox::active),
                "listen", &Amara::MessageBox::listen,
                "send", &Amara::MessageBox::send,
                "isListening", &Amara::MessageBox::isListening,
                "stopListening", sol::overload(
                    sol::resolve<void(const std::string&)>(&Amara::MessageBox::stopListening),
                    sol::resolve<void()>(&Amara::MessageBox::stopListening)
                ),
                "activate", &Amara::MessageBox::activate,
                "deactivate", &Amara::MessageBox::deactivate
            );
        }
    };
}