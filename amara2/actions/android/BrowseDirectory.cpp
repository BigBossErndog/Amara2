namespace Amara {
    struct AndroidBrowseResult {
        std::mutex mtx;
        std::string path;
        bool ok = false;
        std::atomic<bool> done{false};
        std::atomic<bool> pending{false};
    };
    inline AndroidBrowseResult g_androidBrowse;

    extern "C" JNIEXPORT void JNICALL
    Java_com_yourcompany_yourapp_AmaraActivity_nativeOnDirectoryPicked(
        JNIEnv* env, jclass, jstring jpath, jboolean ok)
    {
        std::string path;

        if (jpath != nullptr) {
            const char* c = env->GetStringUTFChars(jpath, nullptr);
            if (c != nullptr) {
                path = c;
                env->ReleaseStringUTFChars(jpath, c);
            } else {
                // JNI may have a pending exception here.
                ok = JNI_FALSE;
            }
        }

        {
            std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
            g_androidBrowse.path = std::move(path);
            g_androidBrowse.ok = (ok == JNI_TRUE);
        }

        g_androidBrowse.done = true;
    }

    class BrowseDirectoryNode: public Amara::Action {
    public:
        BrowseDirectoryNode(): Amara::Action() {
            set_base_node_id("BrowseDirectory");
        }

        virtual void create() override {
            Amara::Action::onCreate();

            if (g_androidBrowse.pending) {
                if (funcs.hasFunction("onFail")) funcs.callFunction("onFail");
                complete();
                return;
            }

            JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
            jobject activity = (jobject)SDL_GetAndroidActivity();
            if (!env || !activity) {
                fatal_error("Error: Could not access Android activity.");
                return;
            }

            jclass cls = env->GetObjectClass(activity);
            jmethodID mid = env->GetMethodID(cls, "browseDirectory", "()V");
            if (!mid) {
                fatal_error("Error: browseDirectory() not found on Android activity.");
                env->DeleteLocalRef(cls);
                env->DeleteLocalRef(activity);
                return;
            }

            g_androidBrowse.done = false;
            g_androidBrowse.pending = true;
            env->CallVoidMethod(activity, mid);

            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();

                g_androidBrowse.pending = false;
                g_androidBrowse.done = false;

                env->DeleteLocalRef(cls);
                env->DeleteLocalRef(activity);

                if (funcs.hasFunction("onFail")) {
                    funcs.callFunction("onFail");
                }

                complete();
                return;
            }

            env->DeleteLocalRef(cls);
            env->DeleteLocalRef(activity);
        }

        virtual void act(double deltaTime) override {
            if (!g_androidBrowse.done.exchange(false)) {
                Amara::Action::act(deltaTime);
                return;
            }

            std::string path;
            bool ok;

            {
                std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
                path = g_androidBrowse.path;
                ok = g_androidBrowse.ok;
            }

            g_androidBrowse.pending = false;

            if (ok) {
                if (funcs.hasFunction("onSuccess")) {
                    funcs.callFunction(
                        actor,
                        "onSuccess",
                        sol::make_object(path)
                    );
                }
            }
            else {
                if (funcs.hasFunction("onFail")) {
                    funcs.callFunction("onFail");
                }
            }

            complete();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<BrowseDirectoryNode>("BrowseDirectory",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>()
            ):
        }
    };
}