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
               log("Error: Failed to read file \"", filePath.c_str(), "\"");
            }
            return "";
        }

        nlohmann::json readJSON(std::string path) {
            if (fileExists(path)) nlohmann::json::parse(readFile(path));
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
			    log("Written file: ", filePath);
				file.write(output.c_str(), output.size());
				file.close();
				return true;
			}
			log("Failed to write to path: ", filePath);
			return false;
        }
        bool luaWriteFile(std::string path, sol::object input) {
            if (input.is<std::string>()) return writeFile(path, input.as<std::string>());
            return writeFile(path, lua_to_json(input));
        }

        bool deleteFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(path)) {
                log("Error: File does not exist: \"", filePath.string(), "\".");
                return false;
            }

            try {
                if (std::filesystem::remove(path)) {
                    log("File deleted successfully: \"", filePath.string(), "\"");
                    return true;
                } else {
                    log("Error: Failed to delete file (unknown reason): \"", filePath.string(), "\".");
                    return false;
                }
            } catch (const std::exception& e) {
                log("Error: Exception while deleting file:  \"", filePath.string(), "\".");
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
                log("Error: \"", filePath.string(), "\" does not exist or is not a directory.");
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        bool createDirectory(std::string path) {
            std::filesystem::path dir = getRelativePath(path);
            std::filesystem::create_directories(dir.parent_path());

            if (!std::filesystem::exists(dir)) {
                if (std::filesystem::create_directory(dir)) {
                    log("Directory created: \"", dir.string(), "\".");
                    return true;
                } else {
                    log("Error: Failed to create directory: \"", dir.string(), "\".");
                }
            } else {
                log("Error: Directory already exists: \"", dir.string(), "\".");
            }
            return false;
        }

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            std::vector<std::string> contents;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(filePath)) {
                log("Error: \"", filePath.string(), "\" does not exist or is not a directory.");
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

        std::string getBasePath() {
            if (basePath.empty()) {
                char* c_basePath = SDL_GetBasePath();
                std::filesystem::path exeDir = c_basePath;
                std::filesystem::path contextPath = Properties::context_path;
                std::filesystem::path finalContext = exeDir / contextPath;
                SDL_free(c_basePath);
                basePath = finalContext.string();
            }
            return basePath;
        }
        std::string setBasePath(std::string path) {
            basePath = path;
            return getBasePath();
        }
        std::string resetBasePath() {
            return setBasePath("");
        }
        
        std::string getRelativePath(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / (std::filesystem::path)path;
            return filePath.lexically_normal().string();
        }

        std::string getScriptPath(std::string path) {
            std::filesystem::path filePath = getRelativePath(Properties::lua_script_path) / (std::filesystem::path)path;
            if (!fileExists(filePath.string())) {
                path = filePath.string() + ".luac";
                if (fileExists(path)) return path;
                path = filePath.string() + ".lua";
                if (fileExists(path)) return path;
            }
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
                    log("Error: \"", source.string(), "\" does not exist.");
                    return false;
                }
                if (fileExists(destination.string())) {
                    if (overwrite) deleteFile(destination.string());
                    else {
                        log("Error: \"", destination.string(), "\" already exists.");
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
                    log("Error: Unable to copy file \"", source.string(), "\" to \"", destination.string(), "\".");
                    return false;
                }
        
                log("Copied \"", source.string(), "\" to \"", destination.string(), "\".");
                return true;
            }
            catch (const std::exception& e) {
                log("Error: Unable to copy file \"", source.string(), "\" to \"", destination.string(), "\".");
            }
            return false;
        }
        bool copy(std::string input, std::string output) {
            return copy(input, output, true);
        }

        sol::object run(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            try {
                return Properties::lua().script_file(filePath.string());
            }
            catch (const sol::error& e) {
                log(e.what());
                throw e;
            }
            return sol::nil;
        }
        sol::load_result load_script(std::string path) {
            std::filesystem::path filePath = getScriptPath(path);
            return Properties::lua().load_file(filePath.string());
        }

        bool compileScript(std::string path, std::string dest) {
            std::filesystem::path filePath = getRelativePath(path);
            if (!fileExists(filePath.string())) {
                log("Error: Script not found \"", filePath.string(), "\".");
                return false;
            }
            sol::load_result script = Properties::lua().load_file(filePath.string());

            if (!script.valid()) {
                sol::error err = script;
                std::cerr << "Error: Failed to load script: " << err.what() << std::endl;
            } else {
                sol::function func = script;

                try {
                    sol::function dump = (Properties::lua())["string"]["dump"];
                    sol::object bytecode = dump(func);
                
                    if (bytecode.is<std::string>()) {
                        std::filesystem::path destPath = getRelativePath(dest);
                        std::filesystem::create_directories(destPath.parent_path());

                        std::string bytecode_str = bytecode.as<std::string>();
                        std::ofstream out(destPath, std::ios::binary);
                        out.write(bytecode_str.data(), bytecode_str.size());
                        out.close();
                        log("Compiled script to \"", destPath.string(), "\"");
                        return true;
                    }
                    else {
                        log("Error: Could not compile script \"", getScriptPath(path), "\"");
                    }
                }
                catch (const sol::error& e) {
                    log("Error: Could not compile script \"", getScriptPath(path), "\"");
                }
            }
            return false;
        }

        template <typename... Args>
        int execute(Args... args) {
            std::ostringstream ss;
            ((ss << args << " && "), ...);
            
            std::string command = ss.str();
            if (!command.empty()) {
                command.erase(command.size() - 4);
            }
            log(command.c_str());
            return std::system(command.c_str());
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
                "execute", &FileManager::lua_execute
            );
        }
    };
}