namespace Amara {
    class SystemManager {
    public:
        std::string basePath;

        Amara::GameProps* gameProps = nullptr;
        sol::object luaobject;

        SystemManager() = default;
        
        bool exists(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            return std::filesystem::exists(filePath);
        }

        std::string readFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            std::string pathForError = removeBasePath(filePath);

            if (!std::filesystem::exists(filePath)) {
                fatal_error("Error: File does not exist \"", pathForError, "\"");
                gameProps->breakWorld();
                return "";
            }

            #if defined(_WIN32)
                std::string filename_str = filePath.filename().string();
                if (!filename_str.empty()) {
                    std::filesystem::path parent_dir = filePath.parent_path();
                    if (!parent_dir.empty() && parent_dir != filePath) {
                        bool case_match = false;
                        for (const auto& entry : std::filesystem::directory_iterator(parent_dir)) {
                            if (entry.path().filename().string() == filename_str) {
                                case_match = true;
                                break;
                            }
                        }
                        if (!case_match) {
                            fatal_error("Error: File does not exist (case-sensitive check failed) \"", pathForError, "\"");
                            gameProps->breakWorld();
                            return "";
                        }
                    }
                }
            #endif
            
            SDL_IOStream *rw = SDL_IOFromFile(filePath.string().c_str(), "rb");
            if (!rw) {
                fatal_error("Error: Failed to open file \"", pathForError, "\": ", SDL_GetError());
                gameProps->breakWorld();
                return "";
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            std::vector<unsigned char> buffer(fileSize);
            if (fileSize > 0) {
                SDL_ReadIO(rw, buffer.data(), fileSize);
            }
            SDL_CloseIO(rw);

            std::string contents = std::string(reinterpret_cast<char*>(buffer.data()), buffer.size());
            
            if (String::startsWith(contents, "_amara_encrypted_")) {
                #if defined(AMARA_ENCRYPTION_KEY)
                contents = decrypt(contents.substr(17), AMARA_STRINGIFY(AMARA_ENCRYPTION_KEY));
                #endif
            }

            return contents;
        }

        nlohmann::json readJSON(std::string path) {
            std::string contents = readFile(path);
            if (!contents.empty() && nlohmann::json::accept(contents)) {
                return nlohmann::json::parse(contents);
            }
            fatal_error("Warning: Invalid JSON file read from \"", removeBasePath(getRelativePath(path)), "\".");
            return nullptr;
        }
        sol::object luaReadJSON(std::string path) {
            return json_to_lua(gameProps->lua, readJSON(path));
        }

        bool writeFile(std::string path, nlohmann::json input, std::string encryptionKey) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!exists(filePath.parent_path().string())) {
                if (!createDirectory(filePath.parent_path().string())) {
                    return false;
                }
            }

            std::string output_str;
            if (input.is_string()) {
                output_str = input.get<std::string>();
            }
            else {
                try {
                    output_str = input.dump(4);
                } catch (const std::exception& e) {
                    fatal_error("Error: Failed to dump JSON to string for writing: ", removeBasePath(filePath), " - ", e.what());
                    return false;
                }
            }

            #if (defined(AMARA_ENCRYPT_OUTPUT) && defined(AMARA_ENCRYPTION_KEY))
            if (encryptionKey.empty()) encryptionKey = AMARA_STRINGIFY(AMARA_ENCRYPTION_KEY);
            #endif
            
            if (!encryptionKey.empty()) {
                output_str = std::string("_amara_encrypted_") + encrypt(output_str, encryptionKey);
            }

            SDL_IOStream* rw = SDL_IOFromFile(filePath.string().c_str(), "wb");
            if (!rw) {
                fatal_error("Error: Failed to open file for writing: ", removeBasePath(filePath), " - ", SDL_GetError());
                return false;
            }

            size_t bytesWritten = 0;
            if (!output_str.empty()) {
                bytesWritten = SDL_WriteIO(rw, output_str.c_str(), output_str.length());
            }

            SDL_CloseIO(rw);
            
            if (bytesWritten != output_str.length()) {
                fatal_error("Error: Failed to finish writing to \"", removeBasePath(filePath), "\".");
                try { std::filesystem::remove(filePath); } catch(...) {}
                return false;
            }

            #if defined(__EMSCRIPTEN__)
            flushPersistentFolder(filePath.parent_path().string());
            #endif

            return true;
        }
        bool writeFile(std::string path, std::string input) {
            return writeFile(path, nlohmann::json::parse(input), "");
        }
        bool luaWriteFile(std::string path, sol::object input, std::string encryptionKey) {
            if (input.is<std::string>()) return writeFile(path, input.as<std::string>(), encryptionKey);
            return writeFile(path, lua_to_json(input), encryptionKey);
        }
        bool luaWriteFile(std::string path, sol::object input) {
            return luaWriteFile(path, input, "");
        }

        bool encryptFile(std::string path, std::string dest, std::string encryptionKey) {
            std::string input = readFile(path);
            return writeFile(dest, input, encryptionKey);
        }

        bool remove(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(filePath)) {
                fatal_error("Error: \"", removeBasePath(filePath), "\" does not exist.");
                return false;
            }

            try {
                if (std::filesystem::is_directory(filePath)) {
                    if (std::filesystem::remove_all(filePath)) {
                        return true;
                    }
                    else {
                        fatal_error("Error: Failed to delete directory \"", removeBasePath(filePath), "\" (unknown reason).");
                        return false;
                    }
                } 
                else {
                    if (std::filesystem::remove(filePath)) {
                        return true;
                    }
                    else {
                        fatal_error("Error: Failed to delete file \"", removeBasePath(filePath), "\" (unknown reason).");
                        return false;
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                fatal_error("Error: Filesystem exception while deleting \"", removeBasePath(filePath), "\": ", e.what());
                return false;
            }
            catch (const std::exception& e) { // Catch other exceptions
                fatal_error("Error: General exception while deleting \"", removeBasePath(filePath), "\": ", e.what());
                return false;
            }
        }

        bool removeDirectoryContents(std::string path) {
            std::filesystem::path dirPath = getRelativePath(path);
            if (!std::filesystem::exists(dirPath)) { 
                fatal_error("Error: \"", removeBasePath(dirPath), "\" does not exist.");
                return false;
            }
            if (!std::filesystem::is_directory(dirPath)) {
                fatal_error("Error: \"", removeBasePath(dirPath), "\" is not a directory.");
                return false;
            }
            try {
                for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                    remove(entry.path().string());
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                fatal_error("Error: Filesystem exception while clearing directory \"", removeBasePath(dirPath), "\": ", e.what());
                return false;
            }
            catch (const std::exception& e) { // Catch other exceptions
                fatal_error("Error: General exception while clearing directory \"", removeBasePath(dirPath), "\": ", e.what());
                return false;
            }

            #if defined(__EMSCRIPTEN__)
            flushPersistentFolder(dirPath.string());
            #endif

            return true;
        }

        bool isDirectory(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            return std::filesystem::is_directory(filePath);
        }

        bool isDirectoryEmpty(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(path)) {
                fatal_error("Error: \"", removeBasePath(filePath), "\" does not exist or is not a directory.");
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        bool createDirectory(const std::string &path) {
            #if !defined(__EMSCRIPTEN__)
                std::filesystem::path dir = getRelativePath(path);
                std::filesystem::create_directories(dir.parent_path());

                if (!std::filesystem::exists(dir)) {
                    if (std::filesystem::create_directory(dir)) return true;
                    else fatal_error("Error: Failed to create directory: \"", removeBasePath(dir), "\".");
                }
                return false;
            #else
                std::filesystem::path dir = path;
                setupPersistentDirectory(dir.string());

                return true;
            #endif
        }

        #if defined(__EMSCRIPTEN__)
        void flushPersistentFolder(const std::string &path) {
            std::filesystem::path dir(path);
            std::filesystem::path parent = dir.parent_path();
            std::string root = parent.empty() ? "/" : "/" + parent.begin()->string();

            EM_ASM_({
                var rootPath = UTF8ToString($0);
                if (FS.analyzePath(rootPath).exists) {
                    var done = false;
                    FS.syncfs(false, function(err) {
                        if (err) console.error("Flush error for", rootPath, err);
                        done = true;
                    });
                    var start = Date.now();
                    while (!done && Date.now() - start < 5000) {} // busy-wait
                }
            }, root.c_str());
        }

        void setupPersistentDirectory(const std::string &path) {
            EM_ASM_({
                var fullPath = UTF8ToString($0);
                var parts = fullPath.split('/').filter(Boolean);
                if (parts.length === 0) return;

                var root = '/' + parts[0];

                // Ensure root exists
                if (!FS.analyzePath(root).exists) FS.mkdir(root);

                // Mount IDBFS if not already
                var rootInfo = FS.analyzePath(root);
                var alreadyPersistent = rootInfo.object && rootInfo.object.mount &&
                                        rootInfo.object.mount.type === FS.filesystems.IDBFS;

                if (!alreadyPersistent) {
                    FS.mount(FS.filesystems.IDBFS, {}, root);
                    var done = false;
                    FS.syncfs(true, function(err) {
                        if (err) console.error("syncfs load error at", root, err);
                        done = true;
                    });
                    var start = Date.now();
                    while (!done && Date.now() - start < 5000) {} // busy-wait
                }

                // Create subfolders
                var cur = root;
                for (var i = 1; i < parts.length; i++) {
                    cur += '/' + parts[i];
                    if (!FS.analyzePath(cur).exists) FS.mkdir(cur);
                }
            }, path.c_str());

            flushPersistentFolder(path);
        }
        #endif

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            std::vector<std::string> contents;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(filePath)) {
                fatal_error("Error: \"", removeBasePath(filePath), "\" does not exist or is not a directory.");
                return contents;
            }

            for (const auto& entry : std::filesystem::directory_iterator(filePath)) {
                contents.push_back(entry.path().lexically_normal().string());
            }
            
            return contents;
        }
        sol::table luaGetDirectoryContents(std::string path) {
            return vector_to_lua(gameProps->lua, getDirectoryContents(path));
        }

        std::vector<std::string> getFilesInDirectory(std::string path) {
            std::filesystem::path dirPath = getRelativePath(path);
            std::vector<std::string> list;

            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    list.push_back(entry.path().string());
                }
            }
        
            return list;
        }
        sol::table luaGetFilesInDirectory(std::string path) {
            return vector_to_lua(gameProps->lua, getFilesInDirectory(path));
        }

        std::vector<std::string> getSubDirectories(std::string path) {
            std::filesystem::path dirPath = getRelativePath(path);
            std::vector<std::string> list;

            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                if (entry.is_directory()) {
                    list.push_back(entry.path().string());
                }
            }
        
            return list;
        }
        sol::table luaGetSubDirectories(std::string path) {
            return vector_to_lua(gameProps->lua, getSubDirectories(path));
        }

        bool clearDirectory(std::string path) {
            std::filesystem::path dirPath = getRelativePath(path);
            try {
                if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
                    std::vector<std::string> contents = getDirectoryContents(dirPath.string());
                    for (const auto& file : contents) {
                        if (std::filesystem::is_regular_file(file)) {
                            remove(file);
                        }
                        else if (std::filesystem::is_directory(file)) {
                            clearDirectory(file);
                        }
                    }
                    return true;
                } else {
                    fatal_error("Error: Cannot clear directory, target path is not a directory \"", removeBasePath(path), "\".");
                }
            } 
            catch (const std::filesystem::filesystem_error& e) {
                fatal_error("Error: Failed to clear directory \"", removeBasePath(path), "\".");
            }
            return false;
        }

        bool equivalent(std::string path1, std::string path2) {
            std::filesystem::path p1 = getRelativePath(path1);
            std::filesystem::path p2 = getRelativePath(path2);
            return p1.lexically_normal() == p2.lexically_normal();
        }

        std::string getBasePath() {
            if (basePath.empty()) {
                const char* c_basePath = SDL_GetBasePath();
                std::filesystem::path exeDir = c_basePath;
                std::filesystem::path contextPath = gameProps->context_path;
                std::filesystem::path finalContext = exeDir / contextPath;
                basePath = finalContext.string();
            }
            return basePath;
        }
        std::string setBasePath(std::string path) {
            resetBasePath();
            basePath = getRelativePath(path);
            return getBasePath();
        }
        std::string resetBasePath() {
            basePath.clear();
            return getBasePath();
        }

        std::string removeBasePath(std::filesystem::path filePath) {
            try {
                return std::filesystem::relative(filePath, getBasePath()).string();
            }
            catch (const std::filesystem::filesystem_error& e) {
                return filePath.string();
            }
            return filePath.string();
        }

        std::string removeBasePath(std::string path) {
            return removeBasePath(std::filesystem::path(path));
        }
        
        std::string getRelativePath(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / (std::filesystem::path)path;
            return filePath.lexically_normal().string();
        }

        std::string getScriptPath(std::string path) {
            std::filesystem::path filePath;
            if (String::endsWith(path, ".lua") || String::endsWith(path, ".luac")) {
                filePath = getRelativePath(gameProps->lua_script_path) / (std::filesystem::path)removeFileExtension(path);
            }
            else {
                filePath = getRelativePath(gameProps->lua_script_path) / (std::filesystem::path)path;
            }

            if (!exists(filePath.string())) {
                path = filePath.string() + ".luac";
                if (exists(path)) return path;
                path = filePath.string() + ".lua";
                if (exists(path)) return path;
                path = filePath.string() + ".amara";
                if (exists(path)) return path;
            }
            return filePath.string();
        }

        std::string getAssetPath(std::string path) {
            std::filesystem::path filePath = getRelativePath(gameProps->assets_path) / (std::filesystem::path)path;
            return filePath.string();
        }

        std::string getFileName(std::string path) {
            return std::filesystem::path(path).filename().string();
        }
        std::string getFileName(std::string path, bool withExtension) {
            if (withExtension) return getFileName(path);
            return removeFileExtension(getFileName(path));
        }
        std::string getDirectoryName(std::string path) {
            return getFileName(path);
        }
        std::string getFileExtension(std::string path) {
            return std::filesystem::path(path).extension().string();
        }
        std::string removeFileExtension(std::string path) {
            return std::filesystem::path(path).replace_extension().string();
        }
        std::string getDirectoryOf(std::string path) {
            return std::filesystem::path(path).parent_path().string();
        }

        std::string getUserArea() {
            #if defined(_WIN32)
                const char* dir = std::getenv("USERPROFILE");
                return dir;
            #elif defined(__linux__)
            #elif defined(__APPLE__)
            #else
                return "";
            #endif
        }

        std::string mergePaths(std::string str1, std::string str2) {
            std::filesystem::path p1(getRelativePath(str1));
            std::filesystem::path p2(getRelativePath(str2));
            
            if (p1.string().find(p2.string()) == 0) {
                return p1.string();
            }
            return p2.string();
        }
        
        std::string lua_join(sol::variadic_args args) {
            std::filesystem::path current;

            std::ostringstream ss;
            for (auto arg : args) {
                current = current / std::filesystem::path(lua_to_string(arg));
            }
            return current.string();
        }

        bool copy(std::string input, std::string output, bool overwrite) {
            std::filesystem::path source = getRelativePath(input);
            std::filesystem::path destination = getRelativePath(output);
            
            try {
                if (!std::filesystem::exists(source)) {
                    fatal_error("Error: Source path \"", removeBasePath(source), "\" does not exist.");
                    return false;
                }

                if (std::filesystem::is_directory(source)) {
                    if (std::filesystem::exists(destination) && !std::filesystem::is_directory(destination)) {
                        fatal_error("Error: Cannot copy a directory to a file path \"", removeBasePath(destination), "\".");
                        return false;
                    }
                    if (!overwrite && std::filesystem::exists(destination)) {
                        fatal_error("Error: Destination path \"", removeBasePath(destination), "\" already exists.");
                        return false;
                    }
                    
                    if (!std::filesystem::exists(destination)) {
                        std::filesystem::create_directories(destination);
                    }

                    for (const auto& entry : std::filesystem::directory_iterator(source)) {
                        const std::filesystem::path& current_source_path = entry.path();
                        const std::filesystem::path& new_dest_path = destination / current_source_path.filename();
                        copy(current_source_path.string(), new_dest_path.string(), overwrite);
                    }
                } 
                else { // Is a file
                    if (std::filesystem::exists(destination) && std::filesystem::is_directory(destination)) {
                        std::filesystem::path file_in_dir = destination / source.filename();
                        if (!overwrite && std::filesystem::exists(file_in_dir)) {
                            fatal_error("Error: Destination file \"", removeBasePath(file_in_dir), "\" already exists.");
                            return false;
                        }
                        std::filesystem::copy_file(source, file_in_dir, std::filesystem::copy_options::overwrite_existing);
                    } 
                    else {
                        if (!overwrite && std::filesystem::exists(destination)) {
                            fatal_error("Error: Destination file \"", removeBasePath(destination), "\" already exists.");
                            return false;
                        }
                        std::filesystem::create_directories(destination.parent_path());
                        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
                    }
                }
                return true;
            } 
            catch (const std::filesystem::filesystem_error& e) {
                fatal_error("Error: ", e.what());
                return false;
            }
        }
        bool copy(std::string input, std::string output) {
            return copy(input, output, true);
        }

        bool rename(std::string filePath, std::string newName) {
            std::filesystem::path source = getRelativePath(filePath);
            if (!std::filesystem::exists(source) || !std::filesystem::is_regular_file(source)) {
                fatal_error("Error: Source file does not exist or is not a file: ", removeBasePath(source));
                return false;
            }

            std::filesystem::path newNamePath(newName);
            std::string newFileName = newNamePath.filename().string();
            if (newNamePath.extension().empty()) {
                newFileName += source.extension().string();
            }

            std::filesystem::path dest = source.parent_path() / newFileName;
            try {
                std::filesystem::rename(source, dest);
                return true;
            } catch (const std::exception& e) {
                fatal_error("Error: Failed to rename file: ", e.what());
                return false;
            }
        }
        
        bool unzip(std::string zipPath, std::string outputDirectory) {
            #if defined(_WIN32)
            std::filesystem::path zipFilePath = getRelativePath(zipPath);
            std::filesystem::path outputPath = getRelativePath(outputDirectory);
            if (!std::filesystem::exists(zipFilePath)) {
                fatal_error("Error: \"", removeBasePath(zipFilePath), "\" does not exist.");
                return false;
            }
            if (!std::filesystem::exists(outputPath)) {
                std::filesystem::create_directories(outputPath);
            }
            std::string cmd = "powershell.exe -Command \"Expand-Archive -Path '" + zipFilePath.string() + "' -DestinationPath '" + outputPath.string() + "' -Force\"";
            int ret = std::system(cmd.c_str());
            if (ret != 0) {
                if (std::filesystem::exists(outputPath)) {
                    std::filesystem::remove_all(outputPath);
                }
                fatal_error("Error: Failed to extract \"", removeBasePath(zipFilePath), "\"");
                return false;
            }
            return true;
            #else
            fatal_error("Error: Unzipping is not supported on this platform.");
            return false;
            #endif
        }

        // Zips the contents of a directory into a zip file (Windows only)
        bool zip(std::string sourceDirectory, std::string targetDirectory, std::string zipFileName) {
            #if defined(_WIN32)
            std::filesystem::path srcDir = getRelativePath(sourceDirectory);
            std::filesystem::path tgtDir = getRelativePath(targetDirectory);
            if (!std::filesystem::exists(srcDir) || !std::filesystem::is_directory(srcDir)) {
                fatal_error("Error: Source directory does not exist or is not a directory: ", removeBasePath(srcDir));
                return false;
            }
            if (!std::filesystem::exists(tgtDir)) {
                std::filesystem::create_directories(tgtDir);
            }
            // Ensure .zip extension
            std::string finalZipFileName = zipFileName;
            if (finalZipFileName.length() < 4 || finalZipFileName.substr(finalZipFileName.length() - 4) != ".zip") {
                finalZipFileName += ".zip";
            }
            std::filesystem::path zipPath = tgtDir / finalZipFileName;
            // Remove existing zip if present
            if (std::filesystem::exists(zipPath)) {
                std::filesystem::remove(zipPath);
            }
            // PowerShell Compress-Archive
            std::string cmd = "powershell.exe -Command \"Compress-Archive -Path '" + srcDir.string() + "\\*' -DestinationPath '" + zipPath.string() + "' -Force\"";
            int ret = std::system(cmd.c_str());
            if (ret != 0) {
                fatal_error("Error: Failed to create zip file: ", removeBasePath(zipPath));
                return false;
            }
            return true;
            #else
            fatal_error("Error: Zipping is not supported on this platform.");
            return false;
            #endif
        }
        bool zip(std::string sourceDirectory, std::string targetDirectory) {
            std::filesystem::path srcDir = getRelativePath(sourceDirectory);
            std::string dirName = srcDir.filename().string();
            return zip(sourceDirectory, targetDirectory, dirName);
        }

        void setCursor(CursorEnum cursor) {
            switch (cursor) {
                case CursorEnum::Default:
                    SDL_SetCursor(gameProps->cursor_default);
                    break;
                case CursorEnum::Pointer:
                    SDL_SetCursor(gameProps->cursor_pointer);
                    break;
                default:
                    fatal_error("Error: Unsupported cursor type.");
                    break;
            }
        }

        sol::object run(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            bool fileExists = std::filesystem::exists(filePath);
            if (!fileExists) {
                fatal_error("Error: Script does not exist \"", removeBasePath(filePath), "\"");
                gameProps->breakWorld();
                return sol::nil;
            }

            std::string scriptContent = readFile(filePath.string());

            if (scriptContent.empty()) {
                fatal_error("Error: Script is empty or could not be read/decrypted \"", removeBasePath(filePath), "\"");
                gameProps->breakWorld();
                return sol::nil;
            }

            std::string chunkFileName;
            std::string filePathStr = filePath.string();
            const std::string luaScriptsDir = "lua_scripts";
            size_t pos = filePathStr.rfind(luaScriptsDir);

            if (pos != std::string::npos) {
                chunkFileName = filePathStr.substr(pos + luaScriptsDir.length());
                if (!chunkFileName.empty() && (chunkFileName.front() == '\\' || chunkFileName.front() == '/')) {
                    chunkFileName.erase(0, 1);
                }
            } 
            else {
                chunkFileName = getFileName(filePath.string());
            }

            try {
                sol::load_result loadResult;
                if (String::endsWith(filePath.string(), ".luac")) {
                    loadResult = gameProps->lua.load(
                        std::string_view(scriptContent.data(), scriptContent.size()), 
                        chunkFileName, sol::load_mode::binary
                    );
                }
                else {                    
                    loadResult = gameProps->lua.load(
                        std::string_view(scriptContent.data(), scriptContent.size()), 
                        chunkFileName, sol::load_mode::text
                    );
                }

                if (!loadResult.valid()) {
                    sol::error err = loadResult;
                    fatal_error(err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                sol::protected_function scriptFunc = loadResult;
                sol::protected_function_result execResult = scriptFunc();

                if (!execResult.valid()) {
                    sol::error err = execResult;
                    fatal_error(err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                return execResult;
            }
            catch (const sol::error& e) {
                fatal_error(e.what());
                gameProps->lua_exception_thrown = true;
                gameProps->breakWorld();
                return sol::nil;
            }
        }

        sol::load_result load_script(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            bool fileExists = std::filesystem::exists(filePath);
            std::string scriptContent = readFile(filePath.string());

            std::string chunkFileName;
            std::string filePathStr = filePath.string();
            const std::string luaScriptsDir = "lua_scripts";
            size_t pos = filePathStr.rfind(luaScriptsDir);
            
            if (pos != std::string::npos) {
                chunkFileName = filePathStr.substr(pos + luaScriptsDir.length());
                if (!chunkFileName.empty() && (chunkFileName.front() == '\\' || chunkFileName.front() == '/')) {
                    chunkFileName.erase(0, 1);
                }
            } 
            else {
                chunkFileName = getFileName(filePath.string());
            }

            if (scriptContent.empty()) {
                if (fileExists) {
                    fatal_error("Error: Script '", chunkFileName, "' is empty or could not be read/decrypted. Cannot execute.");
                }
                else {
                    fatal_error("Error: Script '", chunkFileName, "' does not exist. Cannot execute.");
                }
                gameProps->breakWorld();
            }
            
            if (String::endsWith(filePath.string(), ".luac")) {
                return gameProps->lua.load(
                    std::string_view(scriptContent.data(), scriptContent.size()), 
                    chunkFileName, sol::load_mode::binary
                );
            }
            else {
                return gameProps->lua.load(
                    std::string_view(scriptContent.data(), scriptContent.size()), 
                    chunkFileName, sol::load_mode::text
                );
            }
        }

        bool compileScript(std::string path, std::string dest, std::string encryptionKey) {
            std::filesystem::path filePath = getRelativePath(path);
            if (!exists(filePath.string())) {
                fatal_error("Error: Script not found \"", removeBasePath(filePath), "\".");
                return false;
            }
            sol::load_result script = gameProps->lua.load_file(filePath.string());

            if (!script.valid()) {
                sol::error err = script;
                debug_log(err.what());
            } else {
                sol::function func = script;

                try {
                    sol::function dump = (gameProps->lua)["string"]["dump"];
                    sol::object bytecode = dump(func, true);
                
                    if (bytecode.is<std::string>()) {
                        std::filesystem::path destPath = getRelativePath(dest);
                        std::filesystem::create_directories(destPath.parent_path());

                        std::string bytecode_str = bytecode.as<std::string>();

                        if (writeFile(destPath.string(), bytecode_str, encryptionKey)) {
                            return true;
                        }
                    }
                    else {
                        fatal_error("Error: Could not compile script \"", removeBasePath(filePath), "\"");
                    }
                }
                catch (const sol::error& e) {
                    fatal_error("Error: Could not compile script \"", removeBasePath(filePath), "\"");
                }
            }
            return false;
        }
        bool compileScript(std::string path, std::string dest) {
            return compileScript(path, dest, "");
        }

        void copyToClipboard(std::string text) {
            SDL_SetClipboardText(text.c_str());
        }

        bool openWebsite(std::string url) {
            std::string command;
            int result = -1;

            #if defined(_WIN32)
                #if !defined(AMARA_DEBUG_BUILD)
                    HINSTANCE hinst = ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    result = ((int)hinst > 32) ? 0 : 1;
                #else
                    command = "start \"\" \"" + url + "\"";
                    result = std::system(command.c_str());
                #endif
            #elif defined(__APPLE__)
                command = "open \"" + url + "\"";
                result = std::system(command.c_str());
            #elif defined(__linux__) && !defined(__EMSCRIPTEN__)
                command = "xdg-open \"" + url + "\"";
                result = std::system(command.c_str());
            #elif defined(__EMSCRIPTEN__)
                EM_ASM({
                    window.open(UTF8ToString($0), '_blank');
                }, url.c_str());
                return true;
            #else
                fatal_error("Error: System:openWebsite is not supported on this platform.");
                return false;
            #endif

            #if !defined(__EMSCRIPTEN__)
            if (result != 0) {
                debug_log("Warning: Failed to open website with url: ", url, "'");
                return false;
            }
            return true;
            #endif
        }

        #if defined(__EMSCRIPTEN__)
        void execute(std::string command) {
            EM_ASM({
                window.location.href = UTF8ToString($0);
            }, command.c_str());
        }
        #endif

        #if defined(AMARA_DESKTOP)
        static int run_command(std::string command) {
            #if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
            HINSTANCE result = ShellExecuteA(NULL, "open", "cmd.exe", ("/c " + command).c_str(), NULL, SW_HIDE);
            return ((int)result > 32) ? 0 : 1;
            #else
            return std::system(command.c_str());
            #endif
        }

        template <typename... Args>
        int execute(bool dettached, Args... args) {
            std::ostringstream ss;
            ((ss << args << " && "), ...);
            
            std::string command = ss.str();
            if (!command.empty()) {
                command.erase(command.size() - 4);
            }
            if (!dettached) return run_command(command);
            else {
                std::thread t(run_command, command);
                t.detach();
                return 0;
            }
        }
        int lua_execute(sol::variadic_args args) {
            std::ostringstream ss;
            bool first = true;
            for (auto arg : args) {
                if (!first) {
                    ss << " && ";
                }
                ss << arg.as<std::string>();
                first = false;
            }
            return execute(false, ss.str());
        }
        int lua_executeDettached(sol::variadic_args args) {
            std::ostringstream ss;
            bool first = true;
            for (auto arg : args) {
                if (!first) {
                    ss << " && ";
                }
                ss << arg.as<std::string>();
                first = false;
            }
            return execute(true, ss.str());
        }

        int executeTerminal(std::string command) {
            #if defined(_WIN32)
                HANDLE hStdinRead, hStdinWrite;
                SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
                if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0)) {
                    debug_log("Failed to create pipe for cmd.exe");
                    return -1;
                }

                STARTUPINFOA si = { sizeof(STARTUPINFOA) };
                si.dwFlags |= STARTF_USESTDHANDLES;
                si.hStdInput = hStdinRead;
                si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                PROCESS_INFORMATION pi;
                BOOL success = CreateProcessA(
                    "C:\\Windows\\System32\\cmd.exe",
                    NULL,
                    NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
                );

                if (!success) {
                    debug_log("Failed to start cmd.exe");
                    CloseHandle(hStdinRead);
                    CloseHandle(hStdinWrite);
                    return GetLastError();
                }

                debug_log("Executing command.");

                // Compose the commands to send
                std::string fullCmd;
                // fullCmd += String::concat("echo ", "\"Executing Command\"", "\r\n");
                fullCmd += command + "\r\nexit\r\n";

                DWORD written;
                WriteFile(hStdinWrite, fullCmd.c_str(), (DWORD)fullCmd.size(), &written, NULL);

                WaitForSingleObject(pi.hProcess, INFINITE);

                DWORD exitCode = 0;
                GetExitCodeProcess(pi.hProcess, &exitCode);

                CloseHandle(hStdinRead);
                CloseHandle(hStdinWrite);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                return static_cast<int>(exitCode);
            #elif defined(__APPLE__)
                std::string appleScript = "tell application \"Terminal\" to do script \"" + command + "\"";
                std::string fullCommand = "osascript -e '" + appleScript + "'";
                return std::system(fullCommand.c_str());
            #elif defined(__linux__) && !defined(__EMSCRIPTEN__)
                std::string fullCommand = "x-terminal-emulator -e \"" + command + "\" &";
                return std::system(fullCommand.c_str());
            #else
                fatal_error("Error: System:executeTerminal is not supported on this platform.");
                return -1;
            #endif
        }
        int lua_executeTerminal(sol::variadic_args args) {
            std::ostringstream ss;
            bool first = true;
            for (auto arg : args) {
                if (!first) {
                    ss << " && ";
                }
                ss << arg.as<std::string>();
                first = false;
            }
            std::string command = ss.str();
            return executeTerminal(command);
        }

        bool openDirectory(std::string path) {
            std::string command;
            int result = -1;
            std::string absolutePath = getRelativePath(path);

            #if defined(_WIN32)
                #if !defined(AMARA_DEBUG_BUILD)
                    HINSTANCE hinst = ShellExecuteA(NULL, "explore", absolutePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    result = ((int)hinst > 32) ? 0 : 1;
                #else
                    command = "start \"\" \"" + absolutePath + "\"";
                    result = std::system(command.c_str());
                #endif
            #elif defined(__APPLE__)
                command = "open \"" + absolutePath + "\"";
                result = std::system(command.c_str());
            #elif defined(__linux__) && !defined(__EMSCRIPTEN__)
                command = "xdg-open \"" + absolutePath + "\"";
                result = std::system(command.c_str());
            #else
                fatal_error("Error: System:openDirectory is not supported on this platform.");
                return false;
            #endif
            
            if (result != 0) {
                debug_log("Warning: Failed to open directory: ", removeBasePath(absolutePath));
                return false;
            }
            return true;
        }
        
        std::string browseDirectory() {
            auto path = pfd::select_folder("Select folder").result();
            return path.empty() ? "" : path;
        }
        std::string browseDirectory(std::string defPath) {
            auto path = pfd::select_folder("Select folder", defPath, pfd::opt::force_path).result();
            return path.empty() ? "" : path;
        }

        std::string browseFile() {
            auto result = pfd::open_file("Select a file").result();
            return result.empty() ? "" : result[0];
        }
        std::string browseFile(const std::string& defPath) {
            auto result = pfd::open_file("Select a file", defPath, { "All Files", "*" }, pfd::opt::force_path).result();
            return result.empty() ? "" : result[0];
        }
        std::string browseFile(const std::string& defPath, const std::vector<std::string>& filters) {
            auto result = pfd::open_file("Select a file", defPath, filters, pfd::opt::force_path).result();
            return result.empty() ? "" : result[0];
        }

        bool downloadFile(const std::string& url, const std::string& localPath) {
            #if defined(_WIN32)
                std::wstring wUrl = String::string_to_wstring(url);
                std::wstring wLocalPath = String::string_to_wstring(localPath);

                bool result = false;
                URL_COMPONENTS urlComp = {};
                urlComp.dwStructSize = sizeof(URL_COMPONENTS);
                
                wchar_t hostName[256];
                wchar_t urlPath[1024];
                urlComp.lpszHostName = hostName;
                urlComp.dwHostNameLength = _countof(hostName);
                urlComp.lpszUrlPath = urlPath;
                urlComp.dwUrlPathLength = _countof(urlPath);

                if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp)) {
                    std::cerr << "Invalid URL\n";
                    return false;
                }

                HINTERNET hSession = WinHttpOpen(L"WinHTTP Downloader/1.0",
                                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                                WINHTTP_NO_PROXY_NAME,
                                                WINHTTP_NO_PROXY_BYPASS, 0);
                if (!hSession) return false;

                HINTERNET hConnect = WinHttpConnect(hSession, hostName,
                                                    urlComp.nPort, 0);
                if (!hConnect) {
                    WinHttpCloseHandle(hSession);
                    return false;
                }

                DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
                            WINHTTP_FLAG_SECURE : 0;

                HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET",
                                                        urlPath, nullptr,
                                                        WINHTTP_NO_REFERER,
                                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                        flags);
                if (!hRequest) {
                    WinHttpCloseHandle(hConnect);
                    WinHttpCloseHandle(hSession);
                    return false;
                }

                if (WinHttpSendRequest(hRequest,
                                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    WINHTTP_NO_REQUEST_DATA, 0,
                                    0, 0) &&
                    WinHttpReceiveResponse(hRequest, nullptr)) {
                    
                    std::ofstream outFile(localPath, std::ios::binary);
                    if (!outFile.is_open()) goto cleanup;

                    DWORD bytesRead = 0;
                    BYTE buffer[8192];

                    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                        outFile.write(reinterpret_cast<char*>(buffer), bytesRead);
                    }

                    outFile.close();
                    result = true;
                }

            cleanup:
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return result;
            #endif
        }
        #endif

        #if defined(_WIN32) && defined(AMARA_ENGINE_TOOLS)
        bool VSBuildToolsInstalled() {
            const std::string vswherePath =
                "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe\"";
            const std::string command = vswherePath +
                " -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath";
            
            std::array<char, 256> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);

            if (!pipe) {
                return false;
            }

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }

            result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), result.end());

            return !result.empty();
        }

        bool WriteICO(const std::string& input_path, const std::string& output_path) {
            int width, height, channels;
            
            SDL_IOStream *rw = SDL_IOFromFile(input_path.c_str(), "rb");
            if (!rw) {
                fatal_error("Error: Failed to open file: ", SDL_GetError());
                return false;
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            unsigned char *buffer = (unsigned char*)SDL_malloc(fileSize);
            SDL_ReadIO(rw, buffer, fileSize);
            SDL_CloseIO(rw);

            stbi_set_flip_vertically_on_load(0);

            uint8_t* rgba_pixels = stbi_load_from_memory(buffer, fileSize, &width, &height, &channels, 4);
            SDL_free(buffer);

            if (!rgba_pixels) {
                std::cerr << "Failed to load image: " << removeBasePath(input_path) << "\n";
                return false;
            }

            if (width != 256 || height != 256) {
                std::cerr << "Image must be 256x256 pixels for ICO: " << removeBasePath(input_path) << "\n";
                stbi_image_free(rgba_pixels);
                return false;
            }

            const int bytes_per_pixel = 4;
            const int image_size = width * height * bytes_per_pixel;

            std::ofstream out(output_path, std::ios::binary);
            if (!out) {
                std::cerr << "Failed to write output: " << removeBasePath(output_path) << "\n";
                stbi_image_free(rgba_pixels);
                return false;
            }

            // ICONDIR (6 bytes)
            out.put(0); out.put(0);             // Reserved
            out.put(1); out.put(0);             // Type = Icon
            out.put(1); out.put(0);             // Count = 1

            // ICONDIRENTRY (16 bytes)
            out.put(0);                         // Width = 256 (0 means 256)
            out.put(0);                         // Height = 256
            out.put(0);                         // Color count
            out.put(0);                         // Reserved
            out.put(1); out.put(0);             // Planes
            out.put(32); out.put(0);            // Bits per pixel
            uint32_t bmp_size_pos = static_cast<uint32_t>(out.tellp());
            out.write("\0\0\0\0", 4);           // Size placeholder
            uint32_t bmp_offset = 6 + 16;
            out.write(reinterpret_cast<const char*>(&bmp_offset), 4); // Offset to BMP

            // BITMAPINFOHEADER (40 bytes)
            uint32_t header_size = 40;
            uint32_t dib_width = width;
            uint32_t dib_height = height * 2; // includes AND mask
            uint16_t planes = 1;
            uint16_t bpp = 32;
            uint32_t compression = 0;
            uint32_t image_size_bmp = image_size;
            uint32_t ppm = 2835; // 72 DPI

            out.write(reinterpret_cast<const char*>(&header_size), 4);
            out.write(reinterpret_cast<const char*>(&dib_width), 4);
            out.write(reinterpret_cast<const char*>(&dib_height), 4);
            out.write(reinterpret_cast<const char*>(&planes), 2);
            out.write(reinterpret_cast<const char*>(&bpp), 2);
            out.write(reinterpret_cast<const char*>(&compression), 4);
            out.write(reinterpret_cast<const char*>(&image_size_bmp), 4);
            out.write(reinterpret_cast<const char*>(&ppm), 4);
            out.write(reinterpret_cast<const char*>(&ppm), 4);
            out.write("\0\0\0\0", 4); // Colors used
            out.write("\0\0\0\0", 4); // Important colors

            // Pixel data (BGRA, bottom-up)
            for (int y = height - 1; y >= 0; --y) {
                const uint8_t* row = rgba_pixels + y * width * 4;
                for (int x = 0; x < width; ++x) {
                    out.put(row[x * 4 + 2]); // B
                    out.put(row[x * 4 + 1]); // G
                    out.put(row[x * 4 + 0]); // R
                    out.put(row[x * 4 + 3]); // A
                }
            }

            // AND mask (empty, 1 bit per pixel, padded to 32-bit rows)
            int mask_row_bytes = ((width + 31) / 32) * 4;
            std::vector<uint8_t> mask_row(mask_row_bytes, 0x00);
            for (int y = 0; y < height; ++y) {
                out.write(reinterpret_cast<const char*>(mask_row.data()), mask_row_bytes);
            }

            // Backfill size
            std::streampos file_end = out.tellp();
            uint32_t bmp_size = static_cast<uint32_t>(file_end) - bmp_offset;
            out.seekp(bmp_size_pos);
            out.write(reinterpret_cast<const char*>(&bmp_size), 4);

            stbi_image_free(rgba_pixels);
            return true;
        }
        #endif

        bool programInstalled(std::string programName) {
            std::string command;
            int result;

            #if defined(_WIN32)
                command = "where \"" + programName + "\" > nul 2>&1";
                #if !defined(AMARA_DEBUG_BUILD)
                    std::string params = "/c " + command;

                    SHELLEXECUTEINFOA sei = { sizeof(sei) };
                    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                    sei.hwnd = NULL;
                    sei.lpVerb = "open";
                    sei.lpFile = "cmd.exe";
                    sei.lpParameters = params.c_str();
                    sei.lpDirectory = NULL;
                    sei.nShow = SW_HIDE;
                    sei.hInstApp = NULL;

                    if (!ShellExecuteExA(&sei) || sei.hProcess == NULL) {
                        debug_log("ShellExecuteEx failed to start process for 'where' command.");
                        return false;
                    }

                    WaitForSingleObject(sei.hProcess, INFINITE);

                    DWORD exitCode;
                    GetExitCodeProcess(sei.hProcess, &exitCode);

                    CloseHandle(sei.hProcess);

                    return exitCode == 0;
                #else // AMARA_DEBUG_BUILD on Windows
                    result = std::system(command.c_str());
                    return result == 0;
                #endif
            #elif defined(__linux__) || defined(__APPLE__)
                command = "which \"" + programName + "\" > /dev/null 2>&1";
                result = std::system(command.c_str());
                return result == 0;
            #else
                debug_log("Warning: programInstalled is not fully supported on this platform.");
                return false;
            #endif
        }

        void throwError(std::string error) {
            fatal_error(error);
        }
        void throwError(std::string error, int error_code) {
            gameProps->error_code = error_code;
            fatal_error(error);
        }

        int exit() {
            gameProps->game->hasQuit = true;
            return gameProps->error_code;
        }
        int exit(int code) {
            gameProps->error_code = code;
            return exit();
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<SystemManager>("SystemManager",
                "exists", &SystemManager::exists,
                "readFile", &SystemManager::readFile,
                "readJSON", &SystemManager::luaReadJSON,
                "writeFile", sol::overload(
                    sol::resolve<bool(std::string, sol::object, std::string)>(&SystemManager::luaWriteFile),
                    sol::resolve<bool(std::string, sol::object)>(&SystemManager::luaWriteFile)
                ),
                "encryptFile", &SystemManager::encryptFile,
                "createDirectory", &SystemManager::createDirectory,
                "isDirectory", &SystemManager::isDirectory,
                "directoryExists", &SystemManager::isDirectory,
                "isDirectoryEmpty", &SystemManager::isDirectoryEmpty,
                "getDirectoryContents", &SystemManager::luaGetDirectoryContents,
                "getFilesInDirectory", &SystemManager::luaGetFilesInDirectory,
                "getSubDirectories", &SystemManager::luaGetSubDirectories,
                "equivalent", &SystemManager::equivalent,
                "getBasePath", &SystemManager::getBasePath,
                "setBasePath", &SystemManager::setBasePath,
                "resetBasePath", &SystemManager::resetBasePath,
                "getRelativePath", &SystemManager::getRelativePath,
                "getScriptPath", &SystemManager::getScriptPath,
                "getAssetPath", &SystemManager::getAssetPath,
                "getFileName", sol::overload(
                    sol::resolve<std::string(std::string)>(&SystemManager::getFileName),
                    sol::resolve<std::string(std::string, bool)>(&SystemManager::getFileName)
                ),
                "getDirectoryName", &SystemManager::getDirectoryName,
                "getFileExtension", &SystemManager::getFileExtension,
                "removeFileExtension", &SystemManager::removeFileExtension,
                "getDirectoryOf", &SystemManager::getDirectoryOf,
                "getUserArea", &SystemManager::getUserArea,
                "mergePaths", &SystemManager::mergePaths,
                "join", &SystemManager::lua_join,
                "remove", &SystemManager::remove,
                "removeDirectoryContents", &SystemManager::removeDirectoryContents,
                "copy", sol::overload(
                    sol::resolve<bool(std::string, std::string, bool)>(&SystemManager::copy),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::copy)
                ),
                "rename", &SystemManager::rename,
                "unzip", &SystemManager::unzip,
                "zip", sol::overload(
                    sol::resolve<bool(std::string, std::string, std::string)>(&SystemManager::zip),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::zip)
                ),
                "run", &SystemManager::run,
                "compileScript", sol::overload(
                    sol::resolve<bool(std::string, std::string, std::string)>(&SystemManager::compileScript),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::compileScript)
                ),
                "copyToClipboard", &SystemManager::copyToClipboard,
                #if defined(__EMSCRIPTEN__)
                "execute", &SystemManager::execute,
                #endif
                #if defined(AMARA_DESKTOP)
                "execute", &SystemManager::lua_execute,
                "executeDettached", &SystemManager::lua_executeDettached,
                "executeTerminal", &SystemManager::lua_executeTerminal,
                "openWebsite", &SystemManager::openWebsite,
                "openDirectory", &SystemManager::openDirectory,
                "browseDirectory", sol::overload(
                    sol::resolve<std::string(std::string)>(&SystemManager::browseDirectory),
                    sol::resolve<std::string()>(&SystemManager::browseDirectory)
                ),
                "browseFile", sol::overload(
                    sol::resolve<std::string(const std::string&)>(&SystemManager::browseFile),
                    sol::resolve<std::string()>(&SystemManager::browseFile),
                    [] (SystemManager& s, std::string defPath, sol::object v) {
                        std::vector<std::string> filters;
                        if (v.is<sol::table>()) {
                            sol::table t = v.as<sol::table>();
                            for (int i = 0; i < t.size(); i++) {
                                sol::object obj = t[i];
                                if (obj.is<std::string>()) {
                                    filters.push_back(obj.as<std::string>());
                                }
                            }
                        }
                        else if (v.is<std::string>()) {
                            filters.push_back(v.as<std::string>());
                        }
                        return s.browseFile(defPath, filters);
                    }
                ),
                "downloadFile", &SystemManager::downloadFile,
                #endif
                #if defined(_WIN32) && defined(AMARA_ENGINE_TOOLS)
                "VSBuildToolsInstalled", &SystemManager::VSBuildToolsInstalled,
                "WriteICO", &SystemManager::WriteICO,
                #endif
                "programInstalled", &SystemManager::programInstalled,
                "throwError", sol::overload(
                    sol::resolve<void(std::string)>(&SystemManager::throwError),
                    sol::resolve<void(std::string, int)>(&SystemManager::throwError)
                ),
                "exit", sol::overload(
                    sol::resolve<int()>(&SystemManager::exit),
                    sol::resolve<int(int)>(&SystemManager::exit)
                )
            );
        }
    };
}