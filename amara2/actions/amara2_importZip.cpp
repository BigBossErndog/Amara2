#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#if defined(__ANDROID__)
#include <jni.h>
#endif

namespace Amara {
    struct ImportZipJob {
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
    inline std::mutex g_importZipMtx;
    inline std::shared_ptr<ImportZipJob> g_importZipJob;
    #endif
}

#if defined(__ANDROID__)
extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnImportProgress(JNIEnv*, jclass, jlong done, jlong total)
{
    std::shared_ptr<Amara::ImportZipJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_importZipMtx);
        job = Amara::g_importZipJob;
    }
    if (job) job->setProgress((uint64_t)done, (uint64_t)total);
}

extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnImportFinished(JNIEnv* env, jclass, jint status, jstring jmsg)
{
    std::string msg;
    if (jmsg != nullptr) {
        const char* c = env->GetStringUTFChars(jmsg, nullptr);
        if (c != nullptr) {
            msg = c;
            env->ReleaseStringUTFChars(jmsg, c);
        }
    }

    std::shared_ptr<Amara::ImportZipJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_importZipMtx);
        job = Amara::g_importZipJob;
    }
    if (job) job->finish((int)status, msg);
}
#endif

namespace Amara {
    class ImportZipNode: public Amara::Action {
    public:
        static const int STATUS_OK = 0;
        static const int STATUS_ERROR = 1;
        static const int STATUS_CANCELLED = 2;

        std::string targetPath;
        std::string defPath;
        std::string resolvedTarget;
        std::shared_ptr<ImportZipJob> job;

        #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
        std::unique_ptr<pfd::open_file> dialog;

        struct Sink {
            std::ofstream out;
            ImportZipJob* job = nullptr;
            uint64_t* done = nullptr;
            uint64_t total = 0;
        };

        static size_t sinkWrite(void* opaque, mz_uint64, const void* buf, size_t n) {
            Sink* s = (Sink*)opaque;
            if (s->job->cancel) return 0;
            s->out.write((const char*)buf, (std::streamsize)n);
            if (!s->out) return 0;
            *s->done += n;
            s->job->setProgress(*s->done, s->total);
            return n;
        }

        static void unzipWorker(std::shared_ptr<ImportZipJob> job, std::string zipPath, std::string target) {
            namespace fs = std::filesystem;
            mz_zip_archive zip;
            std::memset(&zip, 0, sizeof(zip));

            try {
                if (!mz_zip_reader_init_file(&zip, zipPath.c_str(), 0)) {
                    job->finish(STATUS_ERROR, "Could not open the zip file.");
                    return;
                }

                auto end = [&](int status, const std::string& msg) {
                    mz_zip_reader_end(&zip);
                    job->finish(status, msg);
                };

                mz_uint count = mz_zip_reader_get_num_files(&zip);
                uint64_t total = 0;
                for (mz_uint i = 0; i < count; i++) {
                    mz_zip_archive_file_stat st;
                    if (mz_zip_reader_file_stat(&zip, i, &st) && !st.m_is_directory) total += st.m_uncomp_size;
                }

                std::error_code ec;
                fs::create_directories(target, ec);
                fs::path root = fs::weakly_canonical(fs::path(target), ec);
                if (ec) { end(STATUS_ERROR, "Invalid target path."); return; }

                uint64_t done = 0;
                for (mz_uint i = 0; i < count; i++) {
                    if (job->cancel) { end(STATUS_CANCELLED, "Cancelled."); return; }

                    mz_zip_archive_file_stat st;
                    if (!mz_zip_reader_file_stat(&zip, i, &st)) { end(STATUS_ERROR, "Corrupt zip file."); return; }

                    fs::path outPath = (root / fs::u8path(st.m_filename)).lexically_normal();
                    fs::path rel = outPath.lexically_relative(root);
                    std::string relStr = rel.generic_string();
                    if (rel.empty() || relStr == ".." || relStr.rfind("../", 0) == 0) {
                        end(STATUS_ERROR, std::string("Unsafe path in zip: ") + st.m_filename);
                        return;
                    }

                    if (st.m_is_directory) {
                        fs::create_directories(outPath, ec);
                        continue;
                    }

                    fs::create_directories(outPath.parent_path(), ec);

                    Sink sink;
                    sink.job = job.get();
                    sink.done = &done;
                    sink.total = total;
                    sink.out.open(outPath, std::ios::binary | std::ios::trunc);
                    if (!sink.out) {
                        end(STATUS_ERROR, std::string("Could not write: ") + st.m_filename);
                        return;
                    }

                    bool ok = mz_zip_reader_extract_to_callback(&zip, i, sinkWrite, &sink, 0);
                    sink.out.close();

                    if (!ok) {
                        fs::remove(outPath, ec);
                        if (job->cancel) end(STATUS_CANCELLED, "Cancelled.");
                        else end(STATUS_ERROR, std::string("Failed to extract: ") + st.m_filename);
                        return;
                    }
                }

                job->setProgress(total, total);
                end(STATUS_OK, "");
            }
            catch (const std::exception& e) {
                mz_zip_reader_end(&zip);
                job->finish(STATUS_ERROR, e.what());
            }
        }
        #endif

        ImportZipNode(): Amara::Action() {
            set_base_node_id("ImportZip");
        }

        ~ImportZipNode() {
            if (job && !job->isFinished()) {
                job->cancel = true;
                #if defined(__ANDROID__)
                requestAndroidCancel();
                #endif
            }
            #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
            if (dialog) {
                dialog->kill();
                dialog.reset();
            }
            #endif
        }

        virtual Amara::Node* configure(nlohmann::json config) override {
            if (json_has(config, "targetPath")) targetPath = json_get<std::string>(config, "targetPath");
            if (json_has(config, "defaultPath")) defPath = json_get<std::string>(config, "defaultPath");
            return Amara::Action::configure(config);
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

        virtual void create() override {
            Amara::Action::create();

            if (targetPath.empty()) {
                reportFinished(STATUS_ERROR, "targetPath is required.");
                return;
            }
            resolvedTarget = resolveStoragePath(targetPath);

            #if defined(__ANDROID__)
            job = std::make_shared<ImportZipJob>();
            bool busy = false;
            {
                std::lock_guard<std::mutex> lock(g_importZipMtx);
                if (g_importZipJob && !g_importZipJob->isFinished()) busy = true;
                else g_importZipJob = job;
            }
            if (busy) {
                job.reset();
                reportFinished(STATUS_ERROR, "Another import is already in progress.");
                return;
            }

            if (!launchAndroid()) {
                {
                    std::lock_guard<std::mutex> lock(g_importZipMtx);
                    if (g_importZipJob == job) g_importZipJob.reset();
                }
                job.reset();
                reportFinished(STATUS_ERROR, "Could not open the file picker.");
            }
            #elif defined(AMARA_DESKTOP)
            job = std::make_shared<ImportZipJob>();
            std::vector<std::string> filters = { "Zip archives", "*.zip", "All files", "*" };
            if (defPath.empty()) {
                dialog = std::make_unique<pfd::open_file>("Select zip file", "", filters);
            }
            else {
                std::string start = SystemManager::global_system->getRelativePath(defPath);
                dialog = std::make_unique<pfd::open_file>("Select zip file", start, filters, pfd::opt::force_path);
            }
            #else
            reportFinished(STATUS_ERROR, "ImportZip is not supported on this platform.");
            #endif
        }

        virtual void act(double deltaTime) override {
            #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
            if (dialog) {
                if (!dialog->ready(0)) {
                    Amara::Action::act(deltaTime);
                    return;
                }
                std::vector<std::string> files = dialog->result();
                dialog.reset();

                if (files.empty()) {
                    job.reset();
                    reportFinished(STATUS_CANCELLED, "Cancelled.");
                    return;
                }

                std::shared_ptr<ImportZipJob> j = job;
                std::string zipPath = files[0];
                std::string target = resolvedTarget;
                std::thread([j, zipPath, target]() { unzipWorker(j, zipPath, target); }).detach();
            }
            #endif

            if (!job) {
                Amara::Action::act(deltaTime);
                return;
            }

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
                return;
            }

            Amara::Action::act(deltaTime);
        }

        void reportProgress(uint64_t done, uint64_t total) {
            if (!funcs.hasFunction("onProgress")) return;
            double fraction = total > 0 ? (double)done / (double)total : 0.0;
            if (fraction > 1.0) fraction = 1.0;
            funcs.callFunction(
                actor, "onProgress",
                sol::make_object(gameProps->lua, fraction),
                sol::make_object(gameProps->lua, (double)done),
                sol::make_object(gameProps->lua, (double)total)
            );
        }

        void reportFinished(int status, const std::string& message) {
            if (status == STATUS_OK) {
                if (funcs.hasFunction("onSuccess")) {
                    funcs.callFunction(actor, "onSuccess", sol::make_object(gameProps->lua, resolvedTarget));
                }
            }
            else if (status == STATUS_CANCELLED && funcs.hasFunction("onCancel")) {
                funcs.callFunction(actor, "onCancel");
            }
            else if (funcs.hasFunction("onFail")) {
                funcs.callFunction(
                    actor, "onFail",
                    sol::make_object(gameProps->lua, status),
                    sol::make_object(gameProps->lua, message)
                );
            }
            complete();
        }

        #if defined(__ANDROID__)
        bool launchAndroid() {
            JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
            jobject activity = (jobject)SDL_GetAndroidActivity();
            BrowseDirectoryNode::LocalRef activityRef(env, activity);

            if (!env || !activity) {
                fatal_error("Error: Could not access Android activity.");
                return false;
            }

            jclass cls = env->GetObjectClass(activity);
            BrowseDirectoryNode::LocalRef clsRef(env, cls);

            jmethodID mid = env->GetMethodID(cls, "importZip", "(Ljava/lang/String;)V");
            if (!mid) {
                env->ExceptionClear();
                fatal_error("Error: importZip(String) not found on Android activity.");
                return false;
            }

            jstring jTarget = env->NewStringUTF(resolvedTarget.c_str());
            BrowseDirectoryNode::LocalRef targetRef(env, jTarget);

            env->CallVoidMethod(activity, mid, jTarget);

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

            jmethodID mid = env->GetMethodID(cls, "cancelImportZip", "()V");
            if (!mid) {
                env->ExceptionClear();
                return;
            }
            env->CallVoidMethod(activity, mid);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        #endif

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ImportZipNode>("ImportZip",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>()
            );
        }
    };
}