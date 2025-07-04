namespace Amara {
    struct GarbageTask {
        Amara::Node* target = nullptr;
        Amara::Asset* asset = nullptr;
        double expiration = 0;
    };

    class GarbageCollector {
    public:
        std::vector<GarbageTask> collection;
        std::vector<GarbageTask> asset_collection;

        std::vector<Amara::Node*> batch_queue;
        std::deque<Amara::Node*> batch_overflow;

        #ifdef AMARA_OPENGL
        std::vector<GLuint> glTextures;
        #endif
        
        int batch_size = 100;

        bool debug = false;

        Amara::GameProps* gameProps = nullptr;

        GarbageCollector() = default;

        void deleteNode(Amara::Node* node) {
            node->props = sol::nil;
            node->luaobject = sol::object(sol::nil);
            delete node;
        }
        
        void clearImmediately() {
            if (debug) debug_log("GarbageCollector: Clearing all ", collection.size(), " ", batch_queue.size(), " ", batch_overflow.size());
            
            lua_gc(gameProps->lua.lua_state(), LUA_GCCOLLECT, 0);
            
            for (GarbageTask& task: collection) {
                deleteNode(task.target);
            }
            for (Amara::Node* node: batch_queue) {
                deleteNode(node);
            }
            for (Amara::Node* node: batch_overflow) {
                deleteNode(node);
            }
            collection.clear();
            batch_queue.clear();
            batch_overflow.clear();

            for (auto it = asset_collection.begin(); it != asset_collection.end();) {
                GarbageTask& task = *it;
                delete task.asset;
                ++it;
            }
            asset_collection.clear();

            #ifdef AMARA_OPENGL
            if (!glTextures.empty()) {
                glDeleteTextures(static_cast<GLsizei>(glTextures.size()), glTextures.data());
                glTextures.clear();
            }
            #endif
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

            for (auto it = asset_collection.begin(); it != asset_collection.end();) {
                GarbageTask& task = *it;
                task.expiration -= deltaTime;
                if (task.expiration <= 0) {
                    delete task.asset;
                    it = collection.erase(it);
                    continue;
                }
                ++it;
            }

            if (batch_queue.size() >= batch_size) {
                if (debug) debug_log("GarbageCollector: Deleting ", batch_queue.size(), " nodes.");
                
                lua_gc(gameProps->lua.lua_state(), LUA_GCCOLLECT, 0);

                for (Amara::Node* node: batch_queue) {
                    deleteNode(node);
                }
                batch_queue.clear();

                while (batch_queue.size() < batch_size && batch_overflow.size() > 0) {
                    batch_queue.push_back(batch_overflow.front());
                    batch_overflow.pop_front();
                }
            }

            #ifdef AMARA_OPENGL
            if (!glTextures.empty()) {
                glDeleteTextures(static_cast<GLsizei>(glTextures.size()), glTextures.data());
                glTextures.clear();
            }
            #endif
        }

        void batch(Amara::Node* node) {
            if (batch_queue.size() > batch_size) {
                batch_overflow.push_back(node);
            }
            else batch_queue.push_back(node);
        }

        void queue(Amara::Node* node, double expiration) {
            if (debug) debug_log("GarbageCollector: deleting node ", *node);
            GarbageTask task;
            task.target = node;
            task.expiration = expiration;
            collection.push_back(task);
        }

        void queue_asset(Amara::Asset* asset, double expiration) {
            if (debug) debug_log("GarbageCollector: deleting asset ", *asset);
            GarbageTask task;
            task.asset = asset;
            task.expiration = expiration;
            asset_collection.push_back(task);
        }

        ~GarbageCollector() {
            clearImmediately();
        }
    };

    void GameProps::queue_garbage(Amara::Node* node, double expiration) {
        if (garbageCollector) garbageCollector->queue(node, expiration);
        else debug_log("Error: Garbage Collector has not been set up. Attempting to delete: ", *node);
    }
    void GameProps::queue_asset_garbage(Amara::Asset* asset, double expiration) {
        if (garbageCollector) garbageCollector->queue_asset(asset, expiration);
        else debug_log("Error: Garbage Collector has not been set up. Attempting to delete Asset: ", *asset);
    }
    #ifdef AMARA_OPENGL
    void GameProps::queue_texture_garbage(GLuint textureID) {
        if (garbageCollector) garbageCollector->glTextures.push_back(textureID);
        else debug_log("Error: Garbage Collector has not been set up. Attempting to delete GL texture.");
    }
    #endif
}