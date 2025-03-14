namespace Amara {
    class FileManager {
    public:
        bool fileExists(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);
            return std::filesystem::exists(filePath);
        }

        std::string readFile(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);

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
                SDL_Log("Failed to read file \"%s\"", filePath.c_str());
            }
            return "";
        }

        nlohmann::json readJSON(std::string path) {
            if (fileExists(path)) nlohmann::json::parse(readFile(path));
        }
        sol::object luaReadJSON(std::string path) {
            return json_to_lua(readJSON(path));
        }

        bool writeFile(std::string input, std::string path) {
            std::filesystem::path filePath = getContextualPath(path);

            std::ofstream file(filePath);
			if (file.fail()) {
				SDL_Log("Failed to write to path: %s", filePath.c_str());
				return false;
			}
			else {
				SDL_Log("Written file: %s", filePath.c_str());
				file << input;
				file.close();
				return true;
			}
			return false;
        }

        bool deleteFile(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);

            if (!std::filesystem::exists(path)) {
                SDL_Log("Error: File does not exist: \"%s\"", filePath.c_str());
                return false;
            }

            try {
                if (std::filesystem::remove(path)) {
                    SDL_Log("Error: File deleted successfully: \"%s\"", filePath.c_str());
                    return true;
                } else {
                    SDL_Log("Error: Failed to delete file (unknown reason): \"%s\"", filePath.c_str());
                    return false;
                }
            } catch (const std::exception& e) {
                SDL_Log("Exception while deleting file:  \"%s\"", filePath.c_str());
                return false;
            }
		    return false;
		}

        bool isDirectory(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);
            return std::filesystem::is_directory(filePath);
        }

        bool isDirectoryEmpty(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(path)) {
                SDL_Log("Error: \"%s\" does not exist or is not a directory.", path.c_str());
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path filePath = getContextualPath(path);

            std::vector<std::string> contents;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(filePath)) {
                SDL_Log("Error: \"%s\" does not exist or is not a directory.", filePath.c_str());
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
            char* basePath = SDL_GetBasePath();
            std::filesystem::path exeDir = basePath;
            SDL_free(basePath);
            return exeDir.string();
        }
        std::string getRelativePath(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / (std::filesystem::path)path;
            return filePath.lexically_normal().string();
        }
        std::string getContextualPath(std::string path) {
            std::filesystem::path scriptPath = GameProperties::context_path;
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / scriptPath / (std::filesystem::path)path;
            return filePath.lexically_normal().string();
        }

        std::string getFileName(std::string path) {
            return std::filesystem::path(path).filename().string();
        }

        sol::object run(std::string path) {
            std::filesystem::path filePath = getContextualPath(GameProperties::lua_script_path) / (std::filesystem::path)path;
            return GameProperties::lua->script_file(filePath.string());
        }

        static void bindLua(sol::state& lua) {
            lua.new_usertype<FileManager>("FileManager",
                "fileExists", &FileManager::fileExists,
                "readFile", &FileManager::readFile,
                "readJSON", &FileManager::luaReadJSON,
                "writeFile", &FileManager::writeFile,
                "deleteFile", &FileManager::deleteFile,
                "isDirectory", &FileManager::isDirectory,
                "isDirectoryEmpty", &FileManager::isDirectoryEmpty,
                "getDirectoryContents", &FileManager::luaGetDirectoryContents,
                "getBasePath", &FileManager::getBasePath,
                "getRelativePath", &FileManager::getRelativePath,
                "getFileName", &FileManager::getFileName,
                "run", &FileManager::run
            );
        }
    };
}