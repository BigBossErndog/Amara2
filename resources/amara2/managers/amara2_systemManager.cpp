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
            
            std::ifstream in(filePath, std::ios::in | std::ios::binary);
            if (in) {
                std::string contents;
                in.seekg(0, std::ios::end);
                contents.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&contents[0], contents.size());
                in.close();
                
                return contents;
            }
            else {
               debug_log("Error: Failed to read file \"", filePath.c_str(), "\"");
            }
            return "";
        }

        nlohmann::json readJSON(std::string path) {
            if (fileExists(path)) return nlohmann::json::parse(readFile(path));
            return nullptr;
        }
        sol::object luaReadJSON(std::string path) {
            return json_to_lua(gameProps->lua, readJSON(path));
        }

        bool writeFile(std::string path, nlohmann::json input) {
            std::filesystem::path filePath = getRelativePath(path);
            std::filesystem::create_directories(filePath.parent_path());

            std::string output;
            if (input.is_string()) output = input;
            else output = input.dump();

            std::ofstream file(filePath);
			if (file.is_open() && !file.fail()) {
			    debug_log("Written file: ", filePath);
				file.write(output.c_str(), output.size());
				file.close();
				return true;
			}
			debug_log("Failed to write to path: ", filePath);
			return false;
        }
        bool luaWriteFile(std::string path, sol::object input) {
            if (input.is<std::string>()) return writeFile(path, input.as<std::string>());
            return writeFile(path, lua_to_json(input));
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
            std::filesystem::path filePath = getRelativePath(path);
            try {
                return gameProps->lua.script_file(filePath.string());
            }
            catch (const sol::error& e) {
                gameProps->lua_exception_thrown = true;
            }
            return sol::nil;
        }
        sol::load_result load_script(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            return gameProps->lua.load_file(filePath.string());
        }

        bool compileScript(std::string path, std::string dest) {
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
                        std::ofstream out(destPath, std::ios::binary);
                        out.write(bytecode_str.data(), bytecode_str.size());
                        out.close();
                        debug_log("Compiled script to \"", destPath.string(), "\"");
                        return true;
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

        static void bindLua(sol::state& lua) {
            lua.new_usertype<SystemManager>("SystemManager",
                "fileExists", &SystemManager::fileExists,
                "readFile", &SystemManager::readFile,
                "readJSON", &SystemManager::luaReadJSON,
                "writeFile", &SystemManager::luaWriteFile,
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
                "compileScript", &SystemManager::compileScript,
                "execute", &SystemManager::lua_execute,
                "setEnvironmentVar", &SystemManager::setEnvironmentVar,
                "openWebsite", &SystemManager::openWebsite
            );
        }
    };
}