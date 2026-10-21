#if defined(__ANDROID__)
#include <jni.h>
#endif
#include <mutex>
#include <string>
#include <memory>

namespace Amara {
    #if defined(__ANDROID__)
    struct AndroidBrowseState {
        std::mutex mtx;
        bool pending = false;
        bool done = false;
        bool ok = false;
        std::string path;
        const void* owner = nullptr;
    };
    inline AndroidBrowseState g_androidBrowse;
    #endif
}

#if defined(__ANDROID__)
extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnDirectoryPicked(
    JNIEnv* env, jclass, jstring jpath, jboolean jok)
{
    std::string path;
    bool ok = (jok == JNI_TRUE);

    if (jpath != nullptr) {
        const char* c = env->GetStringUTFChars(jpath, nullptr);
        if (c != nullptr) {
            path = c;
            env->ReleaseStringUTFChars(jpath, c);
        }
        else {
            ok = false;
        }
    }

    std::lock_guard<std::mutex> lock(Amara::g_androidBrowse.mtx);
    if (!Amara::g_androidBrowse.pending) return;
    Amara::g_androidBrowse.path = std::move(path);
    Amara::g_androidBrowse.ok = ok && !Amara::g_androidBrowse.path.empty();
    Amara::g_androidBrowse.done = true;
}
#endif

namespace Amara {
    class BrowseDirectoryNode: public Amara::Action {
    public:
        std::string defPath;

        #if defined(__ANDROID__)
        struct LocalRef {
            JNIEnv* env;
            jobject obj;
            LocalRef(JNIEnv* e, jobject o): env(e), obj(o) {}
            ~LocalRef() { if (env && obj) env->DeleteLocalRef(obj); }
        };
        #elif defined(AMARA_DESKTOP)
        std::unique_ptr<pfd::select_folder> folderDialog;
        #endif

        BrowseDirectoryNode(): Amara::Action() {
            set_base_node_id("BrowseDirectory");
        }

        ~BrowseDirectoryNode() {
            #if defined(__ANDROID__)
            std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
            if (g_androidBrowse.owner == this) g_androidBrowse.owner = nullptr;
            #elif defined(AMARA_DESKTOP)
            if (folderDialog) {
                folderDialog->kill();
                folderDialog.reset();
            }
            #endif
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "defaultPath")) defPath = json_get<std::string>(config, "defaultPath");
            return Amara::Action::configure(config);
        }
        
        virtual void create() override {
            Amara::Action::create();

            #if defined(__ANDROID__)
            bool busy = false;
            bool adopted = false;
            {
                std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
                if (g_androidBrowse.pending) {
                    if (g_androidBrowse.owner != nullptr) {
                        busy = true;
                    }
                    else if (g_androidBrowse.done) {
                        g_androidBrowse.pending = false;
                        g_androidBrowse.done = false;
                    }
                    else {
                        g_androidBrowse.owner = this;
                        adopted = true;
                    }
                }
            }
            if (adopted) return;
            if (busy) { fail(); return; }

            if (!launchPicker()) { fail(); return; }
            #elif defined(AMARA_DESKTOP)
            if (defPath.empty()) {
                folderDialog = std::make_unique<pfd::select_folder>("Select folder");
            }
            else {
                std::string startPath = defPath;
                if (SystemManager::global_system) {
                    startPath = SystemManager::global_system->getRelativePath(defPath);
                }
                folderDialog = std::make_unique<pfd::select_folder>("Select folder", startPath, pfd::opt::force_path);
            }
            #else
            fail();
            #endif
        }

        virtual void act(double deltaTime) override {
            #if defined(__ANDROID__)
            std::string path;
            bool ok = false;
            bool ready = false;
            {
                std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
                if (g_androidBrowse.pending && g_androidBrowse.done && g_androidBrowse.owner == this) {
                    path = g_androidBrowse.path;
                    ok = g_androidBrowse.ok;
                    g_androidBrowse.pending = false;
                    g_androidBrowse.done = false;
                    g_androidBrowse.owner = nullptr;
                    g_androidBrowse.path.clear();
                    ready = true;
                }
            }
            if (!ready) {
                Amara::Action::act(deltaTime);
                return;
            }

            if (ok) succeed(path);
            else fail();
            #elif defined(AMARA_DESKTOP)
            if (!folderDialog || !folderDialog->ready(0)) {
                Amara::Action::act(deltaTime);
                return;
            }

            std::string path = folderDialog->result();
            folderDialog.reset();

            if (path.empty()) fail();
            else succeed(path);
            #else
            Amara::Action::act(deltaTime);
            #endif
        }

        void fail() {
            if (funcs.hasFunction("onFail")) funcs.callFunction("onFail");
            complete();
        }

        void succeed(const std::string& path) {
            if (funcs.hasFunction("onSuccess")) {
                funcs.callFunction(actor, "onSuccess", sol::make_object(gameProps->lua, path));
            }
            complete();
        }

        #if defined(__ANDROID__)
        bool launchPicker() {
            JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
            jobject activity = (jobject)SDL_GetAndroidActivity();
            LocalRef activityRef(env, activity);

            if (!env || !activity) {
                fatal_error("Error: Could not access Android activity.");
                return false;
            }

            jclass cls = env->GetObjectClass(activity);
            LocalRef clsRef(env, cls);

            jmethodID mid = env->GetMethodID(cls, "browseDirectory", "(Ljava/lang/String;)V");
            if (!mid) {
                env->ExceptionClear();
                fatal_error("Error: browseDirectory(String) not found on Android activity.");
                return false;
            }

            jstring jDefPath = env->NewStringUTF(defPath.c_str());
            LocalRef defPathRef(env, jDefPath);

            {
                std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
                g_androidBrowse.pending = true;
                g_androidBrowse.done = false;
                g_androidBrowse.ok = false;
                g_androidBrowse.path.clear();
                g_androidBrowse.owner = this;
            }

            env->CallVoidMethod(activity, mid, jDefPath);

            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();

                std::lock_guard<std::mutex> lock(g_androidBrowse.mtx);
                g_androidBrowse.pending = false;
                g_androidBrowse.owner = nullptr;
                return false;
            }
            return true;
        }
        #endif

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<BrowseDirectoryNode>("BrowseDirectory",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>()
            );
        }
    };
}