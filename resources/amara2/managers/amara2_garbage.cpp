namespace Amara {
    Amara::Node;

    struct GarbageTask {
        Amara::Node* target = nullptr;
        double expiration = 0;
    };

    class GarbageCollector {
    public:
        std::vector<GarbageTask> collection;
        std::vector<Amara::Node*> batch_queue;
        std::deque<Amara::Node*> batch_overflow;
        
        int batch_size = 100;

        bool debug = false;

        GarbageCollector() {}

        void deleteEntity(Amara::Node* node) {
            node->props = sol::nil;
            node->luaobject = sol::object(sol::nil);
            delete node;
        }
        
        void clearImmediately() {
            if (debug) debug_log("GarbageCollector: Clearing all ", collection.size(), " ", batch_queue.size(), " ", batch_overflow.size());
            
            lua_gc(Props::lua().lua_state(), LUA_GCCOLLECT, 0);

            for (GarbageTask& task: collection) {
                deleteEntity(task.target);
            }
            for (Amara::Node* node: batch_queue) {
                deleteEntity(node);
            }
            for (Amara::Node* node: batch_overflow) {
                deleteEntity(node);
            }
            collection.clear();
            batch_queue.clear();
            batch_overflow.clear();
        }

        void run(double deltaTime) {
            for (auto it = collection.begin(); it != collection.end();) {
                GarbageTask& task = *it;
                task.expiration -= deltaTime;
                if (task.expiration <= 0) {
                    batch(task.target);
                    it = collection.erase(it);
                    continue;
                }
                ++it;
            }

            if (batch_queue.size() >= batch_size) {
                if (debug) debug_log("GarbageCollector: Deleting ", batch_queue.size(), " nodes.");
                
                lua_gc(Props::lua().lua_state(), LUA_GCCOLLECT, 0);

                for (Amara::Node* node: batch_queue) {
                    deleteEntity(node);
                }
                batch_queue.clear();

                while (batch_queue.size() < batch_size && batch_overflow.size() > 0) {
                    batch_queue.push_back(batch_overflow.front());
                    batch_overflow.pop_front();
                }
            }
        }

        void batch(Amara::Node* node) {
            if (batch_queue.size() > batch_size) {
                batch_overflow.push_back(node);
            }
            else batch_queue.push_back(node);
        }

        void queue(Amara::Node* node, double expiration) {
            if (debug) debug_log("GarbageCollector: deleting node ", *node);
            collection.push_back({ node, expiration });
        }

        ~GarbageCollector() {
            clearImmediately();
        }
    };

    void Props::queue_garbage(Amara::Node* node, double expiration) {
        if (Props::garbageCollector) Props::garbageCollector->queue(node, expiration);
        else debug_log("Error: Garbage Collector has not been set up. Attempting to delete: ", *node);
    }
}