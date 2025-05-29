namespace Amara {
    class SystemManager {
    public:
        std::string basePath;

        Amara::GameProps* gameProps = nullptr;

        SystemManager() = default;
        
        bool fileExists(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            return std::filesystem::exists(filePath);
        }

        std::string readFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            
            SDL_IOStream *rw = SDL_IOFromFile(filePath.string().c_str(), "rb");
            if (!rw) {
                debug_log("Failed to open file: ", SDL_GetError());
                gameProps->breakWorld();
                return "";
            }

            Sint64 fileSize = SDL_GetIOSize(rw);
            unsigned char *buffer = (unsigned char*)SDL_malloc(fileSize);
            SDL_ReadIO(rw, buffer, fileSize);
            SDL_CloseIO(rw);

            if (Amara::Encryption::is_buffer_encrypted(buffer, fileSize)) {
                #if defined(AMARA_ENCRYPTION_KEY)
                    Amara::Encryption::decryptBuffer(buffer, fileSize, AMARA_ENCRYPTION_KEY)
                #else
                    debug_log("Error: Attempted to load encrypted data without encryption key. \"", filePath.string(), "\".");
                    SDL_free(buffer);
                    gameProps->breakWorld();
                    return "";
                #endif
            }

            std::string contents(reinterpret_cast<char*>(buffer), fileSize);
            SDL_free(buffer);

            return contents;
        }

        nlohmann::json readJSON(std::string path) {
            if (fileExists(path)) return nlohmann::json::parse(readFile(path));
            return nullptr;
        }
        sol::object luaReadJSON(std::string path) {
            return json_to_lua(gameProps->lua, readJSON(path));
        }

        bool writeFile(std::string path, nlohmann::json input, std::string encryptionKey) {
            std::filesystem::path filePath = getRelativePath(path);

            try {
                 std::filesystem::create_directories(filePath.parent_path());
            } catch (const std::exception& e) {
                debug_log("Error: Failed to create directory for writing: ", filePath.parent_path().string(), " - ", e.what());
                return false;
            }

            std::string output_str;
            if (input.is_string()) {
                output_str = input.get<std::string>();
            } else {
                try {
                    output_str = input.dump(4);
                } catch (const std::exception& e) {
                    debug_log("Error: Failed to dump JSON to string for writing: ", filePath.string(), " - ", e.what());
                    return false;
                }
            }

            const unsigned char* buffer_to_write = nullptr;
            size_t size_to_write = 0;
            std::vector<unsigned char> encrypted_buffer_vec;

            #if (defined(AMARA_ENCRYPT_RW) && defined(AMARA_ENCRYPTION_KEY))
            if (encryptionKey.empty()) encryptionKey = AMARA_ENCRYPTION_KEY;
            #endif

            if (!encryptionKey.empty()) {
                std::vector<unsigned char> data_to_encrypt(output_str.begin(), output_str.end());
                size_t original_size = data_to_encrypt.size();

                if (original_size > 0) {
                    size_t maxEncryptedSize = (original_size + 7) & ~7;
                    maxEncryptedSize += sizeof(Amara::Encryption::ENCRYPTION_HEADER);

                    encrypted_buffer_vec.resize(maxEncryptedSize);

                    std::memcpy(encrypted_buffer_vec.data(), data_to_encrypt.data(), original_size);

                    size_t actualEncryptedSize = original_size;
                    Amara::Encryption::encryptBuffer(encrypted_buffer_vec.data(), actualEncryptedSize, encryptionKey);

                    buffer_to_write = encrypted_buffer_vec.data();
                    size_to_write = actualEncryptedSize;
                } 
                else {
                    buffer_to_write = nullptr;
                    size_to_write = 0;
                }
            }
            else {
                // No encryption, just write the original string data
                buffer_to_write = reinterpret_cast<const unsigned char*>(output_str.data());
                size_to_write = output_str.size();
            }

            SDL_IOStream* rw = SDL_IOFromFile(filePath.string().c_str(), "wb");
            if (!rw) {
                debug_log("Error [writeFile]: Failed to open file for writing: ", filePath.string(), " - ", SDL_GetError());
                return false;
            }

            size_t bytesWritten = 0;
            if (size_to_write > 0 && buffer_to_write != nullptr) {
                 bytesWritten = SDL_WriteIO(rw, buffer_to_write, size_to_write);
            } else {
                bytesWritten = 0;
            }

            SDL_CloseIO(rw);
            
            if (bytesWritten != size_to_write) {
                debug_log("Error: Failed to finish writing to \"", filePath.string(), "\".");
                try { std::filesystem::remove(filePath); } catch(...) {}
                return false;
            }

            debug_log("Written file: ", filePath.string(), " (", size_to_write, " bytes)");
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
            std::filesystem::path filePath = getRelativePath(path);
            if (!fileExists(filePath.string())) {
                debug_log("Error: Input file not found \"", filePath.string(), "\".");
                return false;
            }

            SDL_IOStream *read_rw = SDL_IOFromFile(filePath.string().c_str(), "rb");
            if (!read_rw) {
                debug_log("Error: Failed to open file for reading: ", filePath.string(), " - ", SDL_GetError());
                return false;
            }

            Sint64 fileSize_s64 = SDL_GetIOSize(read_rw);
            if (fileSize_s64 < 0) {
                SDL_CloseIO(read_rw);
                debug_log("Error: Failed to get size of input file: ", filePath.string(), " - ", SDL_GetError());
                return false;
            }
             if (fileSize_s64 == 0) {
                SDL_CloseIO(read_rw);
                debug_log("Warning: Input file is empty from path ", filePath.string());
                return false;
            }
            size_t originalSize = static_cast<size_t>(fileSize_s64);

            size_t maxEncryptedSize = (originalSize + 7) & ~7;
            maxEncryptedSize += sizeof(Amara::Encryption::ENCRYPTION_HEADER);

            unsigned char *buffer = (unsigned char*)SDL_malloc(maxEncryptedSize);
            if (!buffer) {
                SDL_CloseIO(read_rw);
                debug_log("Error: Failed to allocate memory (", maxEncryptedSize, " bytes) for file: \"", filePath.string(), "\".");
                return false;
            }

            size_t bytesRead = SDL_ReadIO(read_rw, buffer, originalSize);
            SDL_CloseIO(read_rw);

            if (bytesRead != originalSize) {
                SDL_free(buffer);
                debug_log("Error: Failed to read entire input file (read ", bytesRead, " of ", originalSize, " bytes): ", filePath.string());
                return false;
            }

            size_t currentSize = originalSize;
            Amara::Encryption::encryptBuffer(buffer, currentSize, encryptionKey);

            std::filesystem::path destPath = getRelativePath(dest);
            try {
                std::filesystem::create_directories(destPath.parent_path());
            } catch (const std::exception& e) {
                SDL_free(buffer);
                debug_log("Error: Failed to create destination directory: ", destPath.parent_path().string(), " - ", e.what());
                return false;
            }

            SDL_IOStream *write_rw = SDL_IOFromFile(destPath.string().c_str(), "wb");
            if (!write_rw) {
                SDL_free(buffer);
                debug_log("Error: Failed to open destination file for writing: ", destPath.string(), " - ", SDL_GetError());
                return false;
            }

            size_t bytesWritten = SDL_WriteIO(write_rw, buffer, currentSize);
            SDL_CloseIO(write_rw);
            SDL_free(buffer);

            if (bytesWritten != currentSize) {
                debug_log("Error: Failed to finish writing to \"", destPath.string(), "\".");
                deleteFile(destPath.string());
                return false;
            }

            debug_log("Successfully encrypted \"", filePath.string(), "\" to \"", destPath.string(), "\"");
            return true;
        }

        bool deleteFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(path)) {
                debug_log("Error: File does not exist: \"", filePath.string(), "\".");
                return false;
            }

            try {
                if (std::filesystem::remove(path)) {
                    debug_log("File deleted successfully: \"", filePath.string(), "\"");
                    return true;
                } else {
                    debug_log("Error: Failed to delete file (unknown reason): \"", filePath.string(), "\".");
                    return false;
                }
            } catch (const std::exception& e) {
                debug_log("Error: Exception while deleting file:  \"", filePath.string(), "\".");
                return false;
            }
		    return false;
		}

        bool isDirectory(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            return std::filesystem::is_directory(filePath);
        }

        bool isDirectoryEmpty(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(path)) {
                debug_log("Error: \"", filePath.string(), "\" does not exist or is not a directory.");
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        bool createDirectory(std::string path) {
            std::filesystem::path dir = getRelativePath(path);
            std::filesystem::create_directories(dir.parent_path());

            if (!std::filesystem::exists(dir)) {
                if (std::filesystem::create_directory(dir)) {
                    debug_log("Directory created: \"", dir.string(), "\".");
                    return true;
                } else {
                    debug_log("Error: Failed to create directory: \"", dir.string(), "\".");
                }
            } else {
                debug_log("Error: Directory already exists: \"", dir.string(), "\".");
            }
            return false;
        }

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            std::vector<std::string> contents;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(filePath)) {
                debug_log("Error: \"", filePath.string(), "\" does not exist or is not a directory.");
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

        bool deleteDirectory(const std::string& path) {
            std::filesystem::path dirPath = getRelativePath(path);
            try {
                if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
                    std::filesystem::remove_all(dirPath);  // Deletes the directory and all its contents
                    return true;
                } else {
                    debug_log("Error: Path does not exist or is not a directory \"", dirPath.string(), "\".");
                }
            } 
            catch (const std::filesystem::filesystem_error& e) {
                debug_log("Error: Failed to delete directory \"", dirPath.string(), "\".");
            }
            return false;
        }

        bool clearDirectory(std::string path) {
            std::filesystem::path dirPath = getRelativePath(path);
            try {
                if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
                    std::vector<std::string> contents = getDirectoryContents(dirPath.string());
                    for (const auto& file : contents) {
                        if (std::filesystem::is_regular_file(file)) {
                            deleteFile(file);
                        }
                        else if (std::filesystem::is_directory(file)) {
                            clearDirectory(file);
                        }
                    }
                    return true;
                } else {
                    debug_log("Error: Cannot clear directory, target path is not a directory \"", path, "\".");
                }
            } 
            catch (const std::filesystem::filesystem_error& e) {
                debug_log("Error: Failed to clear directory \"", path, "\".");
            }
            return false;
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
        
        std::string getRelativePath(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / (std::filesystem::path)path;
            return filePath.lexically_normal().string();
        }

        std::string getScriptPath(std::string path) {
            std::filesystem::path filePath = getRelativePath(gameProps->lua_script_path) / (std::filesystem::path)path;
            if (!fileExists(filePath.string())) {
                path = filePath.string() + ".luac";
                if (fileExists(path)) return path;
                path = filePath.string() + ".lua";
                if (fileExists(path)) return path;
                path = filePath.string() + ".amara";
                if (fileExists(path)) return path;
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
        std::string getDirectoryName(std::string path) {
            return getFileName(path);
        }
        std::string getFileExtension(std::string path) {
            return std::filesystem::path(path).extension().string();
        }
        std::string removeFileExtension(std::string path) {
            return std::filesystem::path(path).replace_extension().string();
        }

        std::string mergePaths(std::string str1, std::string str2) {
            std::filesystem::path p1(getRelativePath(str1));
            std::filesystem::path p2(getRelativePath(str2));

            if (p1.string().find(p2.string()) == 0) {
                return p1.string();
            }
            return p2.string();
        }

        bool copy(std::string input, std::string output, bool overwrite) {
            std::filesystem::path source = getRelativePath(input);
            std::filesystem::path destination = getRelativePath(output);
            try {
                if (!fileExists(source.string())) {
                    debug_log("Error: \"", source.string(), "\" does not exist.");
                    return false;
                }
                if (fileExists(destination.string())) {
                    if (overwrite) deleteFile(destination.string());
                    else {
                        debug_log("Error: \"", destination.string(), "\" already exists.");
                        return false;
                    }
                }
                
                if (std::filesystem::is_regular_file(source)) {
                    std::filesystem::copy_options options = std::filesystem::copy_options::update_existing;
                    std::filesystem::copy(source, destination, options);
                }
                else if (std::filesystem::is_directory(source)) {
                    std::filesystem::copy_options options = std::filesystem::copy_options::recursive;
                    std::filesystem::copy(source, destination, options);
                }
                else {
                    debug_log("Error: Unable to copy file \"", source.string(), "\" to \"", destination.string(), "\".");
                    return false;
                }
        
                debug_log("Copied \"", source.string(), "\" to \"", destination.string(), "\".");
                return true;
            }
            catch (const std::exception& e) {
                debug_log("Error: Unable to copy file \"", source.string(), "\" to \"", destination.string(), "\".");
            }
            return false;
        }
        bool copy(std::string input, std::string output) {
            return copy(input, output, true);
        }

        sol::object run(std::string path) {
            std::string scriptContent = readFile(path);

            std::filesystem::path filePath = getScriptPath(path);
            bool fileExists = std::filesystem::exists(filePath);

            if (scriptContent.empty()) {
                if (fileExists) {
                    debug_log("Error: Script '", path, "' is empty or could not be read/decrypted. Cannot execute.");
                }
                else {
                    debug_log("Error: Script '", path, "' does not exist. Cannot execute.");
                }
                return sol::nil;
            }

            try {
                sol::load_result loadResult = gameProps->lua.load(scriptContent, filePath.string());

                if (!loadResult.valid()) {
                    sol::error err = loadResult;
                    debug_log("Error loading script '", filePath.string(), "': ", err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                sol::protected_function scriptFunc = loadResult;
                sol::protected_function_result execResult = scriptFunc();

                if (!execResult.valid()) {
                    sol::error err = execResult;
                    debug_log("Error: Error while executing script \"", filePath.string(), "\": ", err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                return execResult;
            }
            catch (const sol::error& e) {
                debug_log("Error: Unexpected error during script processing \"", path, "\".");
                debug_log(e.what());
                gameProps->lua_exception_thrown = true;
                gameProps->breakWorld();
                return sol::nil;
            }
        }

        sol::load_result load_script(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            bool fileExists = std::filesystem::exists(filePath);
            std::string scriptContent = readFile(path);

            if (scriptContent.empty()) {
                if (fileExists) {
                    debug_log("Error: Script '", path, "' is empty or could not be read/decrypted. Cannot execute.");
                }
                else {
                    debug_log("Error: Script '", path, "' does not exist. Cannot execute.");
                }
            }

            return gameProps->lua.load(scriptContent, filePath.string());
        }

        bool compileScript(std::string path, std::string dest, std::string encryptionKey) {
            std::filesystem::path filePath = getRelativePath(path);
            if (!fileExists(filePath.string())) {
                debug_log("Error: Script not found \"", filePath.string(), "\".");
                return false;
            }
            sol::load_result script = gameProps->lua.load_file(filePath.string());

            if (!script.valid()) {
                sol::error err = script;
                debug_log("Error: Failed to load script: ", err.what());
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
                            debug_log("Compiled script to \"", destPath.string(), "\"");
                            return true;
                        }
                    }
                    else {
                        debug_log("Error: Could not compile script \"", getScriptPath(path), "\"");
                    }
                }
                catch (const sol::error& e) {
                    debug_log("Error: Could not compile script \"", getScriptPath(path), "\"");
                }
            }
            return false;
        }
        bool compileScript(std::string path, std::string dest) {
            return compileScript(path, dest, "");
        }

        bool execute_blocking = true;

        static int run_command(std::string command) {
            return std::system(command.c_str());
        }

        template <typename... Args>
        int execute(Args... args) {
            std::ostringstream ss;
            ((ss << args << " && "), ...);
            
            std::string command = ss.str();
            if (!command.empty()) {
                command.erase(command.size() - 4);
            }
            debug_log(command.c_str());
            if (execute_blocking) return run_command(command);
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
            return execute(ss.str());
        }

        bool isPathInEnvironment(const std::string& path) {
            #if defined(_WIN32)
                // Get the required buffer size
                DWORD size = GetEnvironmentVariable("PATH", nullptr, 0);
                if (size == 0) {
                    debug_log("Error: Unable to access environment variables.");
                    return false;
                }
                
                // Use std::vector<char> to store the PATH value
                std::vector<char> buffer(size);
                if (GetEnvironmentVariable("PATH", buffer.data(), size) == 0) {
                    debug_log("Error: Unable to access environment variables.");
                    return false;
                }
            
                // Convert buffer to std::string and check if the path exists
                std::string currentPath(buffer.data());
                return currentPath.find(path) != std::string::npos;
            #else
                return false;
            #endif
        }

        bool setEnvironmentVar(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            #if defined(_WIN32)
                if (isPathInEnvironment(filePath.string())) {
                    debug_log("Info: Path \"", filePath.string(), "\" is already set in environment.");
                    return true;
                }
                DWORD size = GetEnvironmentVariable("PATH", nullptr, 0);
                if (size == 0) {
                    debug_log("Error: Unable to access environment variables.");
                    return false;
                }
                std::vector<char> buffer(size);
                if (GetEnvironmentVariable("PATH", buffer.data(), size) == 0) {
                    debug_log("Error: Unable to access environment variables.");
                    return false;
                }

                std::string updatedPath(buffer.data());
                updatedPath += ";" + filePath.string();

                if (!SetEnvironmentVariable("PATH", updatedPath.c_str())) {
                    debug_log("Error: Unable to add new environment variable.");
                    return false;
                }
                    
                return true;
            #endif

            return false;
        }

        bool openWebsite(std::string url) {
            std::string command;
            int result = -1;

            #if defined(_WIN32)
                command = "start \"\" \"" + url + "\"";
                debug_log("SystemManager: Executing command: ", command);
                result = std::system(command.c_str());
            #elif defined(__APPLE__)
                command = "open \"" + url + "\"";
                debug_log("SystemManager: Executing command: ", command);
                result = std::system(command.c_str());
            #elif defined(__linux__) && !defined(__EMSCRIPTEN__)
                command = "xdg-open \"" + url + "\"";
                debug_log("SystemManager: Executing command: ", command);
                result = std::system(command.c_str());
            #elif defined(__EMSCRIPTEN__)
                debug_log("SystemManager: Opening URL via Emscripten: ", url);
                emscripten_open_url_in_new_tab(url.c_str());
                return true;
            #else
                debug_log("Error: System.openWebsite is not supported on this platform.");
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

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<SystemManager>("SystemManager",
                "fileExists", &SystemManager::fileExists,
                "readFile", &SystemManager::readFile,
                "readJSON", &SystemManager::luaReadJSON,
                "writeFile", sol::overload(
                    sol::resolve<bool(std::string, sol::object, std::string)>(&SystemManager::luaWriteFile),
                    sol::resolve<bool(std::string, sol::object)>(&SystemManager::luaWriteFile)
                ),
                "encryptFile", &SystemManager::encryptFile,
                "deleteFile", &SystemManager::deleteFile,
                "createDirectory", &SystemManager::createDirectory,
                "isDirectory", &SystemManager::isDirectory,
                "isDirectoryEmpty", &SystemManager::isDirectoryEmpty,
                "getDirectoryContents", &SystemManager::luaGetDirectoryContents,
                "getFilesInDirectory", &SystemManager::luaGetFilesInDirectory,
                "getSubDirectories", &SystemManager::luaGetSubDirectories,
                "getBasePath", &SystemManager::getBasePath,
                "setBasePath", &SystemManager::setBasePath,
                "resetBasePath", &SystemManager::resetBasePath,
                "getRelativePath", &SystemManager::getRelativePath,
                "getFileName", &SystemManager::getFileName,
                "getDirectoryName", &SystemManager::getDirectoryName,
                "getFileExtension", &SystemManager::getFileExtension,
                "removeFileExtension", &SystemManager::removeFileExtension,
                "mergePaths", &SystemManager::mergePaths,
                "copy", sol::overload(
                    sol::resolve<bool(std::string, std::string, bool)>(&SystemManager::copy),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::copy)
                ),
                "run", &SystemManager::run,
                "compileScript", sol::overload(
                    sol::resolve<bool(std::string, std::string, std::string)>(&SystemManager::compileScript),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::compileScript)
                ),
                "execute", &SystemManager::lua_execute,
                "setEnvironmentVar", &SystemManager::setEnvironmentVar,
                "openWebsite", &SystemManager::openWebsite
            );
        }
    };
}