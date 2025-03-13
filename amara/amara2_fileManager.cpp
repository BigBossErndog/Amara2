namespace Amara {
    class FileManager {
    public:
        bool fileExists(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;
            return std::filesystem::exists(filePath);
        }

        std::string readFile(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;

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
            return nlohmann::json::parse(readFile(path));
        }

        bool writeFile(std::string input, std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;

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
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;

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
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;
            return std::filesystem::is_directory(filePath);
        }

        bool isDirectoryEmpty(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;

            if (!std::filesystem::exists(filePath) || !std::filesystem::is_directory(path)) {
                SDL_Log("Error: \"%s\" does not exist or is not a directory.", path.c_str());
                return false;
            }
        
            return std::filesystem::directory_iterator(path) == std::filesystem::directory_iterator();
        }

        std::vector<std::string> getDirectoryContents(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;

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

        std::string getBasePath() {
            char* basePath = SDL_GetBasePath();
            std::string exeDir = basePath;
            SDL_free(basePath);
            return basePath;
        }

        std::string getFileName(std::string path) {
            return std::filesystem::path(path).filename().string();
        }

        sol::object run(std::string path) {
            std::filesystem::path exeDir = getBasePath();
            std::filesystem::path filePath = exeDir / path;
            return GameProperties::lua->script_file(filePath.string());
        }
    };
}