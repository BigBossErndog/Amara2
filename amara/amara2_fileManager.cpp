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
                c_style_log("Failed to read file \"%s\"", filePath.c_str());
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
            std::cout << output << std::endl;

            std::ofstream file(filePath);
			if (file.fail()) {
				c_style_log("Failed to write to path: %s", filePath.c_str());
				return false;
			}
			else {
				c_style_log("Written file: %s", filePath.c_str());
				file << output;
				file.close();
				return true;
			}
			return false;
        }
        bool luaWriteFile(std::string path, sol::object input) {
            return writeFile(path, lua_to_json(input));
        }

        bool deleteFile(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            if (!std::filesystem::exists(path)) {
                c_style_log("Error: File does not exist: \"%s\"", filePath.c_str());
                return false;
            }

            try {
                if (std::filesystem::remove(path)) {
                    c_style_log("Error: File deleted successfully: \"%s\"", filePath.c_str());
                    return true;
                } else {
                    c_style_log("Error: Failed to delete file (unknown reason): \"%s\"", filePath.c_str());
                    return false;
                }
            } catch (const std::exception& e) {
                c_style_log("Exception while deleting file:  \"%s\"", filePath.c_str());
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
                c_style_log("Error: \"%s\" does not exist or is not a directory.", path.c_str());
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        bool createDirectory(std::string path) {
            std::filesystem::path dir = getRelativePath(path);
            std::filesystem::create_directories(dir.parent_path());

            if (!std::filesystem::exists(dir)) {
                if (std::filesystem::create_directory(dir)) {
                    c_style_log("Directory created: \"%s\"", dir.c_str());
                    return true;
                } else {
                    c_style_log("Failed to create directory: \"%s\"", dir.c_str());
                }
            } else {
                c_style_log("Directory already exists: \"%s\"", dir.c_str());
            }
            return false;
        }

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path filePath = getRelativePath(path);

            std::vector<std::string> contents;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(filePath)) {
                c_style_log("Error: \"%s\" does not exist or is not a directory.", filePath.c_str());
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

        std::string getBasePath() {
            if (basePath.empty()) {
                char* c_basePath = SDL_GetBasePath();
                std::filesystem::path exeDir = c_basePath;
                std::filesystem::path contextPath = GameProperties::context_path;
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

        std::string getFileName(std::string path) {
            return std::filesystem::path(path).filename().string();
        }

        sol::object run(std::string path) {
            std::filesystem::path filePath = getRelativePath(GameProperties::lua_script_path) / (std::filesystem::path)path;
            return GameProperties::lua->script_file(filePath.string());
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<FileManager>("FileManager",
                "fileExists", &FileManager::fileExists,
                "readFile", &FileManager::readFile,
                "readJSON", &FileManager::luaReadJSON,
                "writeFile", &FileManager::luaWriteFile,
                "deleteFile", &FileManager::deleteFile,
                "isDirectory", &FileManager::isDirectory,
                "isDirectoryEmpty", &FileManager::isDirectoryEmpty,
                "getDirectoryContents", &FileManager::luaGetDirectoryContents,
                "getBasePath", &FileManager::getBasePath,
                "setBasePath", &FileManager::setBasePath,
                "resetBasePath", &FileManager::resetBasePath,
                "getRelativePath", &FileManager::getRelativePath,
                "getFileName", &FileManager::getFileName,
                "run", &FileManager::run
            );
        }
    };
}