namespace Amara {
#if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
#include <windows.h>
#include <shellapi.h>
#endif
    class SystemManager {
    public:
        std::string basePath;

        Amara::GameProps* gameProps = nullptr;

        SystemManager() = default;
        
        bool exists(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);
            return std::filesystem::exists(filePath);
        }

        std::string readFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!exists(filePath.string())) {
                debug_log("Error: File does not exist \"", filePath.string(), "\"");
                gameProps->breakWorld();
                return "";
            }
            
            SDL_IOStream *rw = SDL_IOFromFile(filePath.string().c_str(), "rb");
            if (!rw) {
                debug_log("Error: Failed to open file ", SDL_GetError());
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
            if (!exists(path)) {
                debug_log("Error: File does not exist \"", path, "\"");
                return nullptr;
            }
            return nlohmann::json::parse(readFile(path));
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
            if (!exists(filePath.string())) {
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
                remove(destPath.string());
                return false;
            }

            return true;
        }

        bool remove(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(filePath)) {
                debug_log("Error: \"", filePath.string(), "\" does not exist.");
                return false;
            }

            try {
                if (std::filesystem::is_directory(filePath)) {
                    if (std::filesystem::remove_all(filePath)) {
                        return true;
                    }
                    else {
                        debug_log("Error: Failed to delete directory \"", filePath.string(), "\" (unknown reason).");
                        return false;
                    }
                } 
                else {
                    if (std::filesystem::remove(filePath)) {
                        return true;
                    }
                    else {
                        debug_log("Error: Failed to delete file \"", filePath.string(), "\" (unknown reason).");
                        return false;
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                debug_log("Error: Filesystem exception while deleting \"", filePath.string(), "\": ", e.what());
                return false;
            }
            catch (const std::exception& e) { // Catch other exceptions
                debug_log("Error: General exception while deleting \"", filePath.string(), "\": ", e.what());
                return false;
            }
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
                            remove(file);
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
                if (!exists(source.string())) {
                    debug_log("Error: \"", source.string(), "\" does not exist.");
                    return false;
                }
                if (exists(destination.string())) {
                    if (overwrite) remove(destination.string());
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

        void setCursor(CursorEnum cursor) {
            switch (cursor) {
                case CursorEnum::Default:
                    SDL_SetCursor(gameProps->cursor_default);
                    break;
                case CursorEnum::Pointer:
                    SDL_SetCursor(gameProps->cursor_pointer);
                    break;
                default:
                    debug_log("Error: Unsupported cursor type.");
                    break;
            }
        }

        sol::object run(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            bool fileExists = std::filesystem::exists(filePath);

            std::string scriptContent = readFile(filePath.string());

            if (scriptContent.empty()) {
                if (fileExists) {
                    debug_log("Error: Script '", path, "' is empty or could not be read/decrypted. Cannot execute.");
                }
                else {
                    debug_log("Error: Script '", path, "' does not exist. Cannot execute.");
                }
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
                    debug_log("Error loading script \"", filePath.string(), "\"\n", err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                sol::protected_function scriptFunc = loadResult;
                sol::protected_function_result execResult = scriptFunc();

                if (!execResult.valid()) {
                    sol::error err = execResult;
                    debug_log("Error: Error while executing script \"", chunkFileName, "\".\n", err.what());
                    gameProps->lua_exception_thrown = true;
                    gameProps->breakWorld();
                    return sol::nil;
                }

                return execResult;
            }
            catch (const sol::error& e) {
                debug_log("Error: Unexpected error during script processing \"", chunkFileName, "\".");
                debug_log(e.what());
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
                    debug_log("Error: Script '", chunkFileName, "' is empty or could not be read/decrypted. Cannot execute.");
                }
                else {
                    debug_log("Error: Script '", chunkFileName, "' does not exist. Cannot execute.");
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
                        debug_log("Error: Could not compile script \"", filePath.string(), "\"");
                    }
                }
                catch (const sol::error& e) {
                    debug_log("Error: Could not compile script \"", filePath.string(), "\"");
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
        int lua_execute_dettached(sol::variadic_args args) {
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

        bool isPathInEnvironment(const std::string& path) {
            #if defined(_WIN32)
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
                emscripten_open_url_in_new_tab(url.c_str());
                return true;
            #else
                debug_log("Error: System:openWebsite is not supported on this platform.");
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
                debug_log("Error: System:openDirectory is not supported on this platform.");
                return false;
            #endif
            
            if (result != 0) {
                debug_log("Warning: Failed to open directory: ", absolutePath);
                return false;
            }
            return true;
        }
        
        std::string browseDirectory() {
            auto path = pfd::select_folder("Select folder").result();
            return path.empty() ? "" : path;
        }
        std::string browseDirectory(std::string defPath) {
            auto path = pfd::select_folder("Select folder", defPath).result();
            return path.empty() ? "" : path;
        }

        std::string browseFile() {
            auto result = pfd::open_file("Select a file").result();
            return result.empty() ? "" : result[0];
        }
        std::string browseFile(std::string defPath) {
            auto result = pfd::open_file("Select a file", defPath).result();
            return result.empty() ? "" : result[0];
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
                debug_log("Error: Failed to open file: ", SDL_GetError());
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
                std::cerr << "Failed to load image: " << input_path << "\n";
                return false;
            }

            if (width != 256 || height != 256) {
                std::cerr << "Image must be 256x256 pixels for ICO: " << input_path << "\n";
                stbi_image_free(rgba_pixels);
                return false;
            }

            const int bytes_per_pixel = 4;
            const int image_size = width * height * bytes_per_pixel;

            std::ofstream out(output_path, std::ios::binary);
            if (!out) {
                std::cerr << "Failed to write output: " << output_path << "\n";
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
                "getBasePath", &SystemManager::getBasePath,
                "setBasePath", &SystemManager::setBasePath,
                "resetBasePath", &SystemManager::resetBasePath,
                "getRelativePath", &SystemManager::getRelativePath,
                "getScriptPath", &SystemManager::getScriptPath,
                "getFileName", &SystemManager::getFileName,
                "getDirectoryName", &SystemManager::getDirectoryName,
                "getFileExtension", &SystemManager::getFileExtension,
                "removeFileExtension", &SystemManager::removeFileExtension,
                "getDirectoryOf", &SystemManager::getDirectoryOf,
                "mergePaths", &SystemManager::mergePaths,
                "join", &SystemManager::lua_join,
                "remove", &SystemManager::remove,
                "copy", sol::overload(
                    sol::resolve<bool(std::string, std::string, bool)>(&SystemManager::copy),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::copy)
                ),
                "run", &SystemManager::run,
                "compileScript", sol::overload(
                    sol::resolve<bool(std::string, std::string, std::string)>(&SystemManager::compileScript),
                    sol::resolve<bool(std::string, std::string)>(&SystemManager::compileScript)
                ),
                "copyToClipboard", &SystemManager::copyToClipboard,
                #if defined(AMARA_DESKTOP)
                "execute", &SystemManager::lua_execute,
                "execute_dettached", &SystemManager::lua_execute_dettached,
                "setEnvironmentVar", &SystemManager::setEnvironmentVar,
                "openWebsite", &SystemManager::openWebsite,
                "openDirectory", &SystemManager::openDirectory,
                "browseDirectory", sol::overload(
                    sol::resolve<std::string(std::string)>(&SystemManager::browseDirectory),
                    sol::resolve<std::string()>(&SystemManager::browseDirectory)
                ),
                "browseFile", sol::overload(
                    sol::resolve<std::string(std::string)>(&SystemManager::browseFile),
                    sol::resolve<std::string()>(&SystemManager::browseFile)
                ),
                #endif
                #if defined(_WIN32) && defined(AMARA_ENGINE_TOOLS)
                "VSBuildToolsInstalled", &SystemManager::VSBuildToolsInstalled,
                "WriteICO", &SystemManager::WriteICO,
                #endif
                "programInstalled", &SystemManager::programInstalled
            );
        }
    };
}