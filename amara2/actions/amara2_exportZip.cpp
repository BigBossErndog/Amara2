#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#if defined(__ANDROID__)
#include <jni.h>
#endif

namespace Amara {
    struct ExportZipJob {
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
    inline std::mutex g_exportZipMtx;
    inline std::shared_ptr<ExportZipJob> g_exportZipJob;
    #endif
}

#if defined(__ANDROID__)
extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnExportProgress(JNIEnv*, jclass, jlong done, jlong total)
{
    std::shared_ptr<Amara::ExportZipJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_exportZipMtx);
        job = Amara::g_exportZipJob;
    }
    if (job) job->setProgress((uint64_t)done, (uint64_t)total);
}

extern "C" JNIEXPORT void JNICALL
Java_amara_AmaraActivity_nativeOnExportFinished(JNIEnv* env, jclass, jint status, jstring jmsg)
{
    std::string msg;
    if (jmsg != nullptr) {
        const char* c = env->GetStringUTFChars(jmsg, nullptr);
        if (c != nullptr) {
            msg = c;
            env->ReleaseStringUTFChars(jmsg, c);
        }
    }

    std::shared_ptr<Amara::ExportZipJob> job;
    {
        std::lock_guard<std::mutex> lock(Amara::g_exportZipMtx);
        job = Amara::g_exportZipJob;
    }
    if (job) job->finish((int)status, msg);
}
#endif

namespace Amara {
    class ExportZipNode: public Amara::Action {
    public:
        static const int STATUS_OK = 0;
        static const int STATUS_ERROR = 1;
        static const int STATUS_CANCELLED = 2;

        std::string sourcePath;
        std::string fileName;
        std::string defPath;
        std::string resolvedSource;
        std::shared_ptr<ExportZipJob> job;

        #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
        std::unique_ptr<pfd::save_file> dialog;

        static bool isCompressedType(const std::filesystem::path& p) {
            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            static const char* exts[] = { ".png", ".jpg", ".jpeg", ".webp", ".ogg", ".mp3", ".flac", ".zip", ".gz", ".mp4" };
            for (const char* e : exts) if (ext == e) return true;
            return false;
        }

        static void zipWorker(std::shared_ptr<ExportZipJob> job, std::string source, std::string dest) {
            namespace fs = std::filesystem;
            mz_zip_archive zip;
            std::memset(&zip, 0, sizeof(zip));
            bool writerOpen = false;

            try {
                struct Item {
                    fs::path path;
                    std::string name;
                    uint64_t size;
                    bool dir;
                };
                std::vector<Item> items;
                uint64_t total = 0;

                fs::path root = source;
                fs::path destNorm = fs::path(dest).lexically_normal();
                std::error_code ec;

                fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec);
                if (ec) {
                    job->finish(STATUS_ERROR, "Could not read the folder.");
                    return;
                }

                for (; it != fs::recursive_directory_iterator(); it.increment(ec)) {
                    if (ec) {
                        job->finish(STATUS_ERROR, "Could not read the folder.");
                        return;
                    }
                    const fs::directory_entry& e = *it;
                    if (e.path().lexically_normal() == destNorm) continue;

                    std::string name = e.path().lexically_relative(root).generic_string();
                    if (e.is_directory(ec)) {
                        if (fs::is_empty(e.path(), ec)) items.push_back({ e.path(), name + "/", 0, true });
                    }
                    else if (e.is_regular_file(ec)) {
                        uint64_t size = (uint64_t)e.file_size(ec);
                        items.push_back({ e.path(), name, size, false });
                        total += size;
                    }
                }

                if (!mz_zip_writer_init_file(&zip, dest.c_str(), 0)) {
                    job->finish(STATUS_ERROR, "Could not create the zip file.");
                    return;
                }
                writerOpen = true;

                auto abort = [&](int status, const std::string& msg) {
                    mz_zip_writer_end(&zip);
                    writerOpen = false;
                    std::error_code rmEc;
                    fs::remove(dest, rmEc);
                    job->finish(status, msg);
                };

                uint64_t done = 0;
                for (const Item& item : items) {
                    if (job->cancel) { abort(STATUS_CANCELLED, "Cancelled."); return; }

                    bool ok;
                    if (item.dir) {
                        ok = mz_zip_writer_add_mem(&zip, item.name.c_str(), "", 0, MZ_NO_COMPRESSION);
                    }
                    else {
                        mz_uint level = isCompressedType(item.path) ? MZ_NO_COMPRESSION : MZ_DEFAULT_COMPRESSION;
                        ok = mz_zip_writer_add_file(&zip, item.name.c_str(), item.path.string().c_str(), nullptr, 0, level);
                    }
                    if (!ok) { abort(STATUS_ERROR, "Failed to add: " + item.name); return; }

                    done += item.size;
                    job->setProgress(done, total);
                }

                if (!mz_zip_writer_finalize_archive(&zip)) {
                    abort(STATUS_ERROR, "Failed to finish the zip file.");
                    return;
                }
                mz_zip_writer_end(&zip);
                writerOpen = false;

                job->setProgress(total, total);
                job->finish(STATUS_OK, "");
            }
            catch (const std::exception& e) {
                if (writerOpen) {
                    mz_zip_writer_end(&zip);
                    std::error_code rmEc;
                    std::filesystem::remove(dest, rmEc);
                }
                job->finish(STATUS_ERROR, e.what());
            }
        }
        #endif

        ExportZipNode(): Amara::Action() {
            set_base_node_id("ExportZip");
        }

        ~ExportZipNode() {
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
            if (json_has(config, "path")) sourcePath = json_get<std::string>(config, "path");
            if (json_has(config, "fileName")) fileName = json_get<std::string>(config, "fileName");
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

            if (sourcePath.empty()) {
                reportFinished(STATUS_ERROR, "path is required.");
                return;
            }
            resolvedSource = resolveStoragePath(sourcePath);

            std::error_code ec;
            if (!std::filesystem::is_directory(resolvedSource, ec)) {
                reportFinished(STATUS_ERROR, "The folder does not exist.");
                return;
            }

            if (fileName.empty()) fileName = std::filesystem::path(resolvedSource).filename().string();
            if (fileName.empty()) fileName = "export";
            if (fileName.size() < 4 || fileName.substr(fileName.size() - 4) != ".zip") fileName += ".zip";

            #if defined(__ANDROID__)
            job = std::make_shared<ExportZipJob>();
            bool busy = false;
            {
                std::lock_guard<std::mutex> lock(g_exportZipMtx);
                if (g_exportZipJob && !g_exportZipJob->isFinished()) busy = true;
                else g_exportZipJob = job;
            }
            if (busy) {
                job.reset();
                reportFinished(STATUS_ERROR, "Another export is already in progress.");
                return;
            }

            if (!launchAndroid()) {
                {
                    std::lock_guard<std::mutex> lock(g_exportZipMtx);
                    if (g_exportZipJob == job) g_exportZipJob.reset();
                }
                job.reset();
                reportFinished(STATUS_ERROR, "Could not open the save dialog.");
            }
            #elif defined(AMARA_DESKTOP)
            job = std::make_shared<ExportZipJob>();
            std::vector<std::string> filters = { "Zip archives", "*.zip" };
            if (defPath.empty()) {
                dialog = std::make_unique<pfd::save_file>("Export zip", fileName, filters);
            }
            else {
                std::string start = (std::filesystem::path(SystemManager::global_system->getRelativePath(defPath)) / fileName).string();
                dialog = std::make_unique<pfd::save_file>("Export zip", start, filters, pfd::opt::force_path);
            }
            #else
            reportFinished(STATUS_ERROR, "ExportZip is not supported on this platform.");
            #endif
        }

        virtual void act(double deltaTime) override {
            #if defined(AMARA_DESKTOP) && !defined(__ANDROID__)
            if (dialog) {
                if (!dialog->ready(0)) {
                    Amara::Action::act(deltaTime);
                    return;
                }
                std::string dest = dialog->result();
                dialog.reset();

                if (dest.empty()) {
                    job.reset();
                    reportFinished(STATUS_CANCELLED, "Cancelled.");
                    return;
                }
                if (dest.size() < 4 || dest.substr(dest.size() - 4) != ".zip") dest += ".zip";

                std::shared_ptr<ExportZipJob> j = job;
                std::string source = resolvedSource;
                std::thread([j, source, dest]() { zipWorker(j, source, dest); }).detach();
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
                if (funcs.hasFunction("onSuccess")) funcs.callFunction(actor, "onSuccess");
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

            jmethodID mid = env->GetMethodID(cls, "exportZip", "(Ljava/lang/String;Ljava/lang/String;)V");
            if (!mid) {
                env->ExceptionClear();
                fatal_error("Error: exportZip(String, String) not found on Android activity.");
                return false;
            }

            jstring jSource = env->NewStringUTF(resolvedSource.c_str());
            BrowseDirectoryNode::LocalRef sourceRef(env, jSource);
            jstring jName = env->NewStringUTF(fileName.c_str());
            BrowseDirectoryNode::LocalRef nameRef(env, jName);

            env->CallVoidMethod(activity, mid, jSource, jName);

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

            jmethodID mid = env->GetMethodID(cls, "cancelExportZip", "()V");
            if (!mid) {
                env->ExceptionClear();
                return;
            }
            env->CallVoidMethod(activity, mid);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        #endif

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<ExportZipNode>("ExportZip",
                sol::base_classes, sol::bases<Amara::Action, Amara::Node>()
            );
        }
    };
}