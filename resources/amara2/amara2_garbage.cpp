namespace Amara {
    Amara::Entity;

    struct GarbageTask {
        Amara::Entity* target = nullptr;
        double expiration = 0;
    };

    class GarbageCollector {
    public:
        std::vector<GarbageTask> collection;
        std::vector<Amara::Entity*> batch_queue;
        std::deque<Amara::Entity*> batch_overflow;
        
        int batch_size = 10;

        bool debug = true;

        GarbageCollector() {}

        void deleteEntity(Amara::Entity* entity) {
            entity->props = sol::nil;
            entity->luaobject = sol::object(sol::nil);
            delete entity;
        }
        
        void clearImmediately() {
            if (debug) debug_log("GarbageCollector: Clearing all ", collection.size(), " ", batch_queue.size(), " ", batch_overflow.size());
            for (GarbageTask& task: collection) {
                debug_log(*task.target);
                delete task.target;
            }
            debug_log("passed 1");
            for (Amara::Entity* entity: batch_queue) {
                delete entity;
            }
            debug_log("passed 2");
            for (Amara::Entity* entity: batch_overflow) {
                delete entity;
            } 
            debug_log("passed");
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
                if (debug) debug_log("GarbageCollector: Deleting ", batch_queue.size(), " entities.");
                for (Amara::Entity* entity: batch_queue) {
                    delete entity;
                }
                batch_queue.clear();

                while (batch_queue.size() < batch_size && batch_overflow.size() > 0) {
                    batch_queue.push_back(batch_overflow.front());
                    batch_overflow.pop_front();
                }
            }
        }

        void batch(Amara::Entity* entity) {
            if (batch_queue.size() > batch_size) {
                batch_overflow.push_back(entity);
            }
            else batch_queue.push_back(entity);
        }

        void queue(Amara::Entity* entity, double expiration) {
            if (debug) debug_log("GarbageCollector: deleting entity ", *entity);
            collection.push_back({ entity, expiration });
        }

        ~GarbageCollector() {
            clearImmediately();
        }
    };

    void Props::queue_garbage(Amara::Entity* entity, double expiration) {
        if (Props::garbageCollector) Props::garbageCollector->queue(entity, expiration);
        else debug_log("Error: Garbage Collector has not been set up. Attempting to delete: ", *entity);
    }
}