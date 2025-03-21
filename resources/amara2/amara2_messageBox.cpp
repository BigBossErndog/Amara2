namespace Amara {
    class MessageBox {
    public:
        std::unordered_map<std::string, std::vector<sol::function>> messageBox;
    
        void send() {

        }

        void listen(std::string key, sol::function action) {
            if (messageBox.find(key) == messageBox.end()) {
                messageBox[key] = std::vector<sol::function>();
            }
            messageBox[key].push_back(action);
        }

        void run() {
            if (!messageBox.empty()) {
                MessageQueue* messages = Props::messages;

                for (
                    auto it = messages->begin();
                    it != messages->end();
                ) {
                    Message& msg = *it;
                    if (messageBox.find(msg.key) != messageBox.end()) {
                        std::vector<sol::function>& list = messageBox[msg.key];
                        for (sol::function& callback: list) { 
                            callback(*this, json_to_lua(msg.data));
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
    };
}