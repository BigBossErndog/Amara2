namespace Amara {
    class MessageBox;
    
    struct Message {
        Amara::MessageBox* sender = nullptr;
        std::string key;
        sol::object data = sol::nil;
        
        bool active = true;
        bool null = false;
        bool skip = false;
        bool forceRemove = false;

        bool is(const std::string& check) {
            if (key.compare(check) == 0) return true;
            return false;
        }
    };

    class MessageQueue {
    public:
        std::list<Message> queue;

        static Message nullMessage;
        
        MessageQueue() {}
        
        void update() {
            for (auto it = queue.begin(); it != queue.end();) {
                Message msg = *it;
                if (msg.sender == nullptr || msg.forceRemove) {
                    if (msg.skip) msg.skip = false;
                    else {
                        it = queue.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }

        auto begin() {
            return queue.begin();
        }
        auto end() {
            return queue.end();
        }

        void clear() {
            queue.clear();
        }

        bool empty() {
            return (queue.size() == 0);
        }

        int size() {
            return queue.size();
        }

        Message& get(std::string key) {
            for (auto it = queue.begin(); it != queue.end(); ++it) {
                Message& msg = *it;
                if (msg.active && key.compare(msg.key) == 0) {
                    return msg;
                }
            }
            return nullMessage;
        }

        Message& send(std::string key, sol::object _msgData) {
            queue.push_back({ nullptr, key, _msgData });
            return queue.back();
        }

        Message& send(Amara::MessageBox* gParent, std::string key, sol::object _msgData) {
            queue.push_back({ gParent, key, _msgData });
            return queue.back();
        }
    };
    Message MessageQueue::nullMessage = { nullptr, "null", {}, false, true, true };
}