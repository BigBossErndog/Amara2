namespace Amara {
    class FileManager {
    public:
        std::string basePath;

        FileManager() {
            getBasePath();
        }

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
            return json_to_lua(readJSON(path));
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
            return vector_to_lua(getDirectoryContents(path));
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
            return vector_to_lua(getFilesInDirectory(path));
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
            return vector_to_lua(getSubDirectories(path));
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
            } catch (const std::filesystem::filesystem_error& e) {
                debug_log("Error: Failed to delete directory \"", dirPath.string(), "\".");
            }
            return false;
        }

        std::string getBasePath() {
            if (basePath.empty()) {
                const char* c_basePath = SDL_GetBasePath();
                std::filesystem::path exeDir = c_basePath;
                std::filesystem::path contextPath = Props::context_path;
                std::filesystem::path finalContext = exeDir / contextPath;
                basePath = finalContext.string();
            }
            return basePath;
        }
        std::string setBasePath(std::string path) {
            basePath = path;
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
            std::filesystem::path filePath = getRelativePath(Props::lua_script_path) / (std::filesystem::path)path;
            if (!fileExists(filePath.string())) {
                path = filePath.string() + ".luac";
                if (fileExists(path)) return path;
                path = filePath.string() + ".lua";
                if (fileExists(path)) return path;
            }
            return filePath.string();
        }

        std::string getAssetPath(std::string path) {
            std::filesystem::path filePath = getRelativePath(Props::assets_path) / (std::filesystem::path)path;
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
                return Props::lua().script_file(filePath.string());
            }
            catch (const sol::error& e) {
                Props::lua_exception_thrown = true;
            }
            return sol::nil;
        }
        sol::load_result load_script(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            return Props::lua().load_file(filePath.string());
        }

        bool compileScript(std::string path, std::string dest) {
            std::filesystem::path filePath = getRelativePath(path);
            if (!fileExists(filePath.string())) {
                debug_log("Error: Script not found \"", filePath.string(), "\".");
                return false;
            }
            sol::load_result script = Props::lua().load_file(filePath.string());

            if (!script.valid()) {
                sol::error err = script;
                debug_log("Error: Failed to load script: ", err.what());
            } else {
                sol::function func = script;

                try {
                    sol::function dump = (Props::lua())["string"]["dump"];
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

        bool isPathInRegistry(const std::string& path) {
            #if defined(_WIN32) 
                HKEY hKey;
                const char* regPath = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
            
                // Open registry key
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                    debug_log("Error: Unable to access environment variables.");
                    return false;
                }
            
                // Get required buffer size
                DWORD bufferSize = 0;
                if (RegQueryValueEx(hKey, "Path", nullptr, nullptr, nullptr, &bufferSize) != ERROR_SUCCESS) {
                    debug_log("Error: Unable to access environment variables.");
                    RegCloseKey(hKey);
                    return false;
                }
            
                // Use vector for dynamic buffer
                std::vector<char> buffer(bufferSize);
                if (RegQueryValueEx(hKey, "Path", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.data()), &bufferSize) != ERROR_SUCCESS) {
                    debug_log("Error: Unable to access environment variables.");
                    RegCloseKey(hKey);
                    return false;
                }
            
                // Close registry key
                RegCloseKey(hKey);
            
                // Convert to std::string and check if the path exists
                std::string currentPath(buffer.data());
                return currentPath.find(path) != std::string::npos;
            #else
                return false;
            #endif
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

        bool setEnvironmentVar(std::string path, bool permanent) {
            std::filesystem::path filePath = getRelativePath(path);
            #if defined(_WIN32)
                if (isPathInRegistry(filePath.string())) {
                    debug_log("Info: Path \"", filePath.string(), "\" is already set in registry.");
                    return true;
                }
                if (isPathInEnvironment(filePath.string())) {
                    debug_log("Info: Path \"", filePath.string(), "\" is already set in environment.");
                    return true;
                }
                if (permanent) {
                    HKEY hKey;
                    const char* regPath = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
                    const std::string newPath = getRelativePath(path);
                    // Open the registry key
                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
                        debug_log("Error: Unable to access environment variables.");
                        return 1;
                    }

                    // Get the required buffer size
                    DWORD bufferSize = 0;
                    if (RegQueryValueEx(hKey, "Path", nullptr, nullptr, nullptr, &bufferSize) != ERROR_SUCCESS) {
                        debug_log("Error: Unable to access environment variables.");
                        RegCloseKey(hKey);
                        return false;
                    }

                    // Use vector as a dynamically sized buffer
                    std::vector<char> currentPath(bufferSize);
                    if (RegQueryValueEx(hKey, "Path", nullptr, nullptr, reinterpret_cast<LPBYTE>(currentPath.data()), &bufferSize) != ERROR_SUCCESS) {
                        debug_log("Error: Unable to access environment variables.");
                        RegCloseKey(hKey);
                        return false;
                    }

                    // Convert to std::string and append new path
                    std::string updatedPath(currentPath.data());
                    updatedPath += ";" + newPath;

                    // Set the new PATH value
                    if (RegSetValueEx(hKey, "Path", 0, REG_EXPAND_SZ, reinterpret_cast<const BYTE*>(updatedPath.c_str()), updatedPath.size() + 1) != ERROR_SUCCESS) {
                        debug_log("Error: Unable to add new environment variable.");
                        RegCloseKey(hKey);
                        return false;
                    }

                    // Close the registry key
                    RegCloseKey(hKey);

                    // Notify the system of the change
                    SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>("Environment"));
                    
                    return true;
                }
                else {
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
                }
                    
                return true;
            #endif

            return false;
        }

        bool setEnvironmentVar(std::string path) {
            return setEnvironmentVar(path, false);
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<FileManager>("FileManager",
                "fileExists", &FileManager::fileExists,
                "readFile", &FileManager::readFile,
                "readJSON", &FileManager::luaReadJSON,
                "writeFile", &FileManager::luaWriteFile,
                "deleteFile", &FileManager::deleteFile,
                "createDirectory", &FileManager::createDirectory,
                "isDirectory", &FileManager::isDirectory,
                "isDirectoryEmpty", &FileManager::isDirectoryEmpty,
                "getDirectoryContents", &FileManager::luaGetDirectoryContents,
                "getFilesInDirectory", &FileManager::luaGetFilesInDirectory,
                "getSubDirectories", &FileManager::luaGetSubDirectories,
                "getBasePath", &FileManager::getBasePath,
                "setBasePath", &FileManager::setBasePath,
                "resetBasePath", &FileManager::resetBasePath,
                "getRelativePath", &FileManager::getRelativePath,
                "getFileName", &FileManager::getFileName,
                "getDirectoryName", &FileManager::getDirectoryName,
                "getFileExtension", &FileManager::getFileExtension,
                "removeFileExtension", &FileManager::removeFileExtension,
                "mergePaths", &FileManager::mergePaths,
                "copy", sol::overload(
                    sol::resolve<bool(std::string, std::string, bool)>(&FileManager::copy),
                    sol::resolve<bool(std::string, std::string)>(&FileManager::copy)
                ),
                "run", &FileManager::run,
                "compileScript", &FileManager::compileScript,
                "execute", &FileManager::lua_execute,
                "setEnvironmentVar", sol::overload(
                    sol::resolve<bool(std::string, bool)>(&FileManager::setEnvironmentVar),
                    sol::resolve<bool(std::string)>(&FileManager::setEnvironmentVar)
                )
            );
        }
    };
}