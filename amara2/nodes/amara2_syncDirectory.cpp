namespace Amara {
    struct SyncJob {
        std::mutex mtx;
        bool finished = false;
        bool progressDirty = false;
        int status = 1;
        std::string message;
        uint64_t done = 0;
        uint64_t total = 0;
        std::atomic<bool> cancel{false};

        void setProgress(uint64_t d, uint64_t t) {
            std::lock_guard<std::mutex> lock(mtx);
            done = d;
            total = t;
            progressDirty = true;
        }

        void finish(int s, const std::string& m) {
            std::lock_guard<std::mutex> lock(mtx);
            if (finished) return;
            status = s;
            message = m;
            finished = true;
        }

        bool isFinished() {
            std::lock_guard<std::mutex> lock(mtx);
            return finished;
        }
    };

    #if defined(__ANDROID__)
    inline std::mutex g_syncMtx;
    inline std::shared_ptr<SyncJob> g_syncJob;
    #endif
}

#if defined(__ANDROID__)
extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnSyncProgress(JNIEnv*, jclass, jlong done, jlong total)
{
    std::shared_ptr<Amara::SyncJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_syncMtx);
        job = Amara::g_syncJob;
    }
    if (job) job->setProgress((uint64_t)done, (uint64_t)total);
}

extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnSyncFinished(JNIEnv* env, jclass, jint status, jstring jmsg)
{
    std::string msg;
    if (jmsg != nullptr) {
        const char* c = env->GetStringUTFChars(jmsg, nullptr);
        if (c != nullptr) {
            msg = c;
            env->ReleaseStringUTFChars(jmsg, c);
        }
    }

    std::shared_ptr<Amara::SyncJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_syncMtx);
        job = Amara::g_syncJob;
    }
    if (job) job->finish((int)status, msg);
}
#endif

namespace Amara {
    class SyncDirectoryNode: public Amara::Node {
    public:
        static const int STATUS_OK = 0;
        static const int STATUS_ERROR = 1;
        static const int STATUS_CANCELLED = 2;

        static constexpr const char* TMP_SUFFIX = ".amara_tmp";

        std::string targetPath;
        std::string localPath;
        bool mirror = false;
        bool force = false;

        std::string direction;
        std::shared_ptr<SyncJob> job;

        sol::protected_function callOnSuccess;
        sol::protected_function callOnFail;
        sol::protected_function callOnCancel;
        sol::protected_function callOnProgress;

        #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
        static bool pathsOverlap(const std::filesystem::path& a, const std::filesystem::path& b) {
            std::filesystem::path rel = b.lexically_relative(a);
            if (rel.empty()) return false;
            return rel == "." || *rel.begin() != "..";
        }

        static bool endsWithTmp(const std::string& name) {
            std::string suffix = TMP_SUFFIX;
            return name.size() >= suffix.size()
                && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        static int copyFileChunked(SyncJob* job, const std::filesystem::path& from,
                                   const std::filesystem::path& to, uint64_t& done, uint64_t total) {
            namespace fs = std::filesystem;
            fs::path tmp = to;
            tmp += TMP_SUFFIX;
            std::error_code ec;

            {
                std::ifstream in(from, std::ios::binary);
                if (!in) return STATUS_ERROR;
                std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
                if (!out) return STATUS_ERROR;

                std::vector<char> buf(256 * 1024);
                while (in) {
                    in.read(buf.data(), (std::streamsize)buf.size());
                    std::streamsize n = in.gcount();
                    if (n <= 0) break;

                    if (job->cancel) {
                        out.close();
                        fs::remove(tmp, ec);
                        return STATUS_CANCELLED;
                    }

                    out.write(buf.data(), n);
                    if (!out) {
                        out.close();
                        fs::remove(tmp, ec);
                        return STATUS_ERROR;
                    }
                    done += (uint64_t)n;
                    job->setProgress(done, total);
                }

                bool readFailed = in.bad();
                out.close();
                if (readFailed || out.fail()) {
                    fs::remove(tmp, ec);
                    return STATUS_ERROR;
                }
            }

            fs::rename(tmp, to, ec);
            if (ec) {
                std::error_code ec2;
                fs::remove(to, ec2);
                ec.clear();
                fs::rename(tmp, to, ec);
                if (ec) {
                    fs::remove(tmp, ec2);
                    return STATUS_ERROR;
                }
            }
            return STATUS_OK;
        }

        static void syncWorker(std::shared_ptr<SyncJob> job, std::string srcStr, std::string dstStr, bool mirror, bool force) {
            namespace fs = std::filesystem;
            try {
                std::error_code ec;
                fs::path src = fs::weakly_canonical(fs::path(srcStr), ec);
                if (ec || !fs::is_directory(src, ec)) {
                    job->finish(STATUS_ERROR, "The source folder does not exist.");
                    return;
                }
                ec.clear();
                fs::path dst = fs::weakly_canonical(fs::path(dstStr), ec);
                if (ec) {
                    job->finish(STATUS_ERROR, "Invalid destination path.");
                    return;
                }
                if (pathsOverlap(src, dst) || pathsOverlap(dst, src)) {
                    job->finish(STATUS_ERROR, "The two folders must not be the same or inside each other.");
                    return;
                }

                struct Item {
                    std::string rel;
                    fs::path path;
                    uint64_t size;
                    bool dir;
                    fs::file_time_type mtime;
                };
                std::vector<Item> items;
                uint64_t total = 0;

                fs::recursive_directory_iterator it(src, fs::directory_options::skip_permission_denied, ec);
                fs::recursive_directory_iterator end;
                if (ec) {
                    job->finish(STATUS_ERROR, "Could not read the source folder.");
                    return;
                }
                for (; it != end; it.increment(ec)) {
                    if (ec) {
                        job->finish(STATUS_ERROR, "Could not read the source folder.");
                        return;
                    }
                    if (job->cancel) {
                        job->finish(STATUS_CANCELLED, "Cancelled.");
                        return;
                    }

                    const fs::directory_entry& e = *it;
                    std::string rel = e.path().lexically_relative(src).generic_string();

                    std::error_code eec;
                    if (e.is_directory(eec)) {
                        items.push_back({ rel, e.path(), 0, true, fs::file_time_type() });
                    }
                    else if (e.is_regular_file(eec)) {
                        if (endsWithTmp(e.path().filename().string())) continue;
                        uint64_t size = (uint64_t)e.file_size(eec);
                        fs::file_time_type mt = e.last_write_time(eec);
                        items.push_back({ rel, e.path(), size, false, mt });
                        total += size;
                    }
                }

                fs::create_directories(dst, ec);
                if (ec) {
                    job->finish(STATUS_ERROR, "Could not create the destination folder.");
                    return;
                }

                std::unordered_set<std::string> expected;
                for (const Item& item : items) expected.insert(item.rel);

                uint64_t done = 0;
                for (const Item& item : items) {
                    if (job->cancel) {
                        job->finish(STATUS_CANCELLED, "Cancelled.");
                        return;
                    }

                    fs::path out = dst / fs::path(item.rel);
                    std::error_code eec;

                    if (item.dir) {
                        if (fs::exists(out, eec) && !fs::is_directory(out, eec)) fs::remove(out, eec);
                        fs::create_directories(out, eec);
                        if (eec) {
                            job->finish(STATUS_ERROR, "Could not create folder: " + item.rel);
                            return;
                        }
                        continue;
                    }

                    if (fs::is_directory(out, eec)) fs::remove_all(out, eec);

                    bool copy = force;
                    if (!copy) {
                        if (!fs::is_regular_file(out, eec)) {
                            copy = true;
                        }
                        else if ((uint64_t)fs::file_size(out, eec) != item.size) {
                            copy = true;
                        }
                        else {
                            fs::file_time_type destTime = fs::last_write_time(out, eec);
                            copy = eec || (destTime + std::chrono::seconds(2) < item.mtime);
                        }
                    }

                    if (!copy) {
                        done += item.size;
                        job->setProgress(done, total);
                        continue;
                    }

                    fs::create_directories(out.parent_path(), eec);
                    int result = copyFileChunked(job.get(), item.path, out, done, total);
                    if (result == STATUS_CANCELLED) {
                        job->finish(STATUS_CANCELLED, "Cancelled.");
                        return;
                    }
                    if (result != STATUS_OK) {
                        job->finish(STATUS_ERROR, "Failed to copy: " + item.rel);
                        return;
                    }
                    fs::last_write_time(out, item.mtime, eec);
                }

                if (mirror) {
                    std::vector<fs::path> doomed;
                    fs::recursive_directory_iterator dit(dst, fs::directory_options::skip_permission_denied, ec);
                    fs::recursive_directory_iterator dend;
                    for (; !ec && dit != dend; dit.increment(ec)) {
                        std::string name = dit->path().filename().string();
                        if (endsWithTmp(name)) continue;
                        std::string rel = dit->path().lexically_relative(dst).generic_string();
                        if (expected.find(rel) == expected.end()) {
                            doomed.push_back(dit->path());
                            std::error_code eec;
                            if (dit->is_directory(eec)) dit.disable_recursion_pending();
                        }
                    }
                    for (const fs::path& p : doomed) {
                        if (job->cancel) {
                            job->finish(STATUS_CANCELLED, "Cancelled.");
                            return;
                        }
                        std::error_code rec;
                        fs::remove_all(p, rec);
                    }
                }

                job->setProgress(total, total);
                job->finish(STATUS_OK, "");
            }
            catch (const std::exception& e) {
                job->finish(STATUS_ERROR, e.what());
            }
        }
        #endif

        SyncDirectoryNode(): Amara::Node() {
            set_base_node_id("SyncDirectory");
        }

        ~SyncDirectoryNode() {
            cancelJob();
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "targetPath")) targetPath = json_get<std::string>(config, "targetPath");
            if (json_has(config, "localPath")) localPath = json_get<std::string>(config, "localPath");
            if (json_has(config, "mirror")) mirror = json_get<bool>(config, "mirror");
            if (json_has(config, "force")) force = json_get<bool>(config, "force");
            return Amara::Node::configure(config);
        }

        virtual void destroy() override {
            cancelJob();
            clearCallbacks();
            Amara::Node::destroy();
        }

        virtual void memory_clean_up() override {
            clearCallbacks();
            Amara::Node::memory_clean_up();
        }

        void clearCallbacks() {
            callOnSuccess = sol::protected_function();
            callOnFail = sol::protected_function();
            callOnCancel = sol::protected_function();
            callOnProgress = sol::protected_function();
        }

        std::string resolveStoragePath(const std::string& p) {
            std::filesystem::path fp(p);
            if (fp.is_absolute()) return fp.lexically_normal().string();

            #if defined(__ANDROID__)
            std::filesystem::path base;
            if (!gameProps->define_org.empty() && !gameProps->define_app.empty()) {
                char* raw = SDL_GetPrefPath(gameProps->define_org.c_str(), gameProps->define_app.c_str());
                if (raw) {
                    base = raw;
                    SDL_free(raw);
                }
            }
            if (base.empty()) {
                const char* internal = SDL_GetAndroidInternalStoragePath();
                if (internal) base = internal;
            }
            if (!base.empty()) return (base / fp).lexically_normal().string();
            #endif

            return SystemManager::global_system->getRelativePath(p);
        }

        bool syncing() {
            return job != nullptr;
        }

        bool luaPush(sol::object opts) { return start(true, opts); }
        bool luaPull(sol::object opts) { return start(false, opts); }
        void luaCancel() { cancelJob(); }

        void cancelJob() {
            if (job && !job->isFinished()) {
                job->cancel = true;
                #if defined(__ANDROID__)
                requestAndroidCancel();
                #endif
            }
        }

        static sol::protected_function readCallback(sol::table& t, const char* key) {
            sol::object o = t[key];
            if (o.valid() && o.is<sol::function>()) return o.as<sol::protected_function>();
            return sol::protected_function();
        }

        bool start(bool push, sol::object opts) {
            if (job) return false;

            clearCallbacks();

            bool useMirror = mirror;
            bool useForce = force;
            if (opts.valid() && opts.is<sol::table>()) {
                sol::table t = opts.as<sol::table>();
                sol::object m = t["mirror"];
                sol::object f = t["force"];
                if (m.valid() && m.is<bool>()) useMirror = m.as<bool>();
                if (f.valid() && f.is<bool>()) useForce = f.as<bool>();

                callOnSuccess = readCallback(t, "onSuccess");
                callOnFail = readCallback(t, "onFail");
                callOnCancel = readCallback(t, "onCancel");
                callOnProgress = readCallback(t, "onProgress");
            }

            direction = push ? "push" : "pull";
            job = std::make_shared<SyncJob>();

            if (localPath.empty()) {
                job->finish(STATUS_ERROR, "localPath is required.");
                return true;
            }
            std::string resolvedLocal = resolveStoragePath(localPath);

            #if defined(__ANDROID__)
            bool busy = false;
            {
                std::lock_guard<std::mutex> lock(g_syncMtx);
                if (g_syncJob && !g_syncJob->isFinished()) busy = true;
                else g_syncJob = job;
            }
            if (busy) {
                job->finish(STATUS_ERROR, "Another sync is already in progress.");
                return true;
            }

            std::string target = (targetPath.rfind("content://", 0) == 0) ? targetPath : "";
            if (!launchAndroid(resolvedLocal, target, push, useMirror, useForce)) {
                {
                    std::lock_guard<std::mutex> lock(g_syncMtx);
                    if (g_syncJob == job) g_syncJob.reset();
                }
                job->finish(STATUS_ERROR, "Could not start the sync.");
            }
            #elif defined(AMARA_DESKTOP)
            if (targetPath.empty()) {
                job->finish(STATUS_ERROR, "targetPath is required.");
                return true;
            }
            std::string resolvedTarget = resolveStoragePath(targetPath);

            std::shared_ptr<SyncJob> j = job;
            std::string from = push ? resolvedLocal : resolvedTarget;
            std::string to = push ? resolvedTarget : resolvedLocal;
            std::thread([j, from, to, useMirror, useForce]() {
                syncWorker(j, from, to, useMirror, useForce);
            }).detach();
            #else
            job->finish(STATUS_ERROR, "SyncDirectory is not supported on this platform.");
            #endif

            return true;
        }

        virtual void update(double deltaTime) override {
            if (!job) return;

            bool dirty = false;
            bool finished = false;
            uint64_t done = 0, total = 0;
            int status = STATUS_ERROR;
            std::string message;
            {
                std::lock_guard<std::mutex> lock(job->mtx);
                dirty = job->progressDirty;
                job->progressDirty = false;
                done = job->done;
                total = job->total;
                finished = job->finished;
                status = job->status;
                message = job->message;
            }

            if (dirty) reportProgress(done, total);

            if (finished) {
                job.reset();
                reportFinished(status, message);
            }
        }

        template <typename... Args>
        void invokeCallback(sol::protected_function& fn, Args&&... args) {
            sol::protected_function_result result = fn(get_lua_object(), std::forward<Args>(args)...);
            if (!result.valid()) {
                sol::error err = result;
                fatal_error(err.what());
                gameProps->breakWorld();
            }
        }

        void reportProgress(uint64_t done, uint64_t total) {
            double fraction = total > 0 ? (double)done / (double)total : 0.0;
            if (fraction > 1.0) fraction = 1.0;

            if (callOnProgress.valid()) {
                sol::protected_function cb = callOnProgress;
                invokeCallback(cb, fraction, (double)done, (double)total);
                return;
            }
            if (!funcs.hasFunction("onProgress")) return;
            funcs.callFunction(
                "onProgress",
                sol::make_object(gameProps->lua, fraction),
                sol::make_object(gameProps->lua, (double)done),
                sol::make_object(gameProps->lua, (double)total)
            );
        }

        void reportFinished(int status, const std::string& message) {
            sol::protected_function onSuccess = callOnSuccess;
            sol::protected_function onFail = callOnFail;
            sol::protected_function onCancel = callOnCancel;
            clearCallbacks();

            if (status == STATUS_OK) {
                if (onSuccess.valid()) {
                    invokeCallback(onSuccess, direction);
                }
                else if (funcs.hasFunction("onSuccess")) {
                    funcs.callFunction("onSuccess", sol::make_object(gameProps->lua, direction));
                }
            }
            else if (status == STATUS_CANCELLED && (onCancel.valid() || funcs.hasFunction("onCancel"))) {
                if (onCancel.valid()) invokeCallback(onCancel);
                else funcs.callFunction("onCancel");
            }
            else {
                if (onFail.valid()) {
                    invokeCallback(onFail, status, message);
                }
                else if (funcs.hasFunction("onFail")) {
                    funcs.callFunction(
                        "onFail",
                        sol::make_object(gameProps->lua, status),
                        sol::make_object(gameProps->lua, message)
                    );
                }
            }
        }

        #if defined(__ANDROID__)
        bool launchAndroid(const std::string& local, const std::string& target,
                           bool push, bool useMirror, bool useForce) {
            JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
            jobject activity = (jobject)SDL_GetAndroidActivity();
            BrowseDirectoryNode::LocalRef activityRef(env, activity);

            if (!env || !activity) {
                fatal_error("Error: Could not access Android activity.");
                return false;
            }

            jclass cls = env->GetObjectClass(activity);
            BrowseDirectoryNode::LocalRef clsRef(env, cls);

            jmethodID mid = env->GetMethodID(cls, "startSync", "(Ljava/lang/String;Ljava/lang/String;ZZZ)V");
            if (!mid) {
                env->ExceptionClear();
                fatal_error("Error: startSync(String, String, boolean, boolean, boolean) not found on Android activity.");
                return false;
            }

            jstring jLocal = env->NewStringUTF(local.c_str());
            BrowseDirectoryNode::LocalRef localRef(env, jLocal);
            jstring jTarget = env->NewStringUTF(target.c_str());
            BrowseDirectoryNode::LocalRef targetRef(env, jTarget);

            env->CallVoidMethod(
                activity, mid, jLocal, jTarget,
                (jboolean)(push ? JNI_TRUE : JNI_FALSE),
                (jboolean)(useMirror ? JNI_TRUE : JNI_FALSE),
                (jboolean)(useForce ? JNI_TRUE : JNI_FALSE)
            );

            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                return false;
            }
            return true;
        }

        static void requestAndroidCancel() {
            JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
            jobject activity = (jobject)SDL_GetAndroidActivity();
            BrowseDirectoryNode::LocalRef activityRef(env, activity);
            if (!env || !activity) return;

            jclass cls = env->GetObjectClass(activity);
            BrowseDirectoryNode::LocalRef clsRef(env, cls);

            jmethodID mid = env->GetMethodID(cls, "cancelSyncDirectory", "()V");
            if (!mid) {
                env->ExceptionClear();
                return;
            }
            env->CallVoidMethod(activity, mid);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        #endif

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<SyncDirectoryNode>("SyncDirectory",
                sol::base_classes, sol::bases<Amara::Node>(),
                "push", &SyncDirectoryNode::luaPush,
                "pull", &SyncDirectoryNode::luaPull,
                "cancel", &SyncDirectoryNode::luaCancel,
                "syncing", sol::property(&SyncDirectoryNode::syncing),
                "targetPath", &SyncDirectoryNode::targetPath,
                "localPath", &SyncDirectoryNode::localPath,
                "mirror", &SyncDirectoryNode::mirror,
                "force", &SyncDirectoryNode::force
            );
        }
    };
}