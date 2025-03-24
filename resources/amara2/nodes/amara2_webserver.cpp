#if defined(_WIN32)
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    #define INIT_SOCKETS()  do { WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa); } while(0)
    #define CLEANUP_SOCKETS() WSACleanup()
    typedef int socklen_t;
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    #define INIT_SOCKETS()
    #define CLEANUP_SOCKETS()
#else
    #error "Unsupported OS"
#endif

namespace Amara {
    class WebServer: public Node {
    public:
        std::atomic<bool> server_running{false};
        SDL_Thread* server_thread = nullptr;
        int port;
        std::string base_path;

        // MIME type map
        std::unordered_map<std::string, std::string> mime_types = {
            {".html", "text/html"},
            {".css", "text/css"},
            {".js", "application/javascript"},
            {".wasm", "application/wasm"},
            {".data", "application/octet-stream"},
            {".png", "image/png"},
            {".jpg", "image/jpeg"},
            {".gif", "image/gif"},
            {".ico", "image/x-icon"},
            {".ogg", "audio/ogg"},
            {".mp3", "audio/mpeg"},
            {".lua", "text/plain"},
            {".json", "application/json"}
        };

        WebServer(const std::string& base_folder, int port_number)
            : Node(), base_path(base_folder), port(port_number) {}

        std::string get_requested_file(const std::string& request) {
            std::istringstream stream(request);
            std::string method, path;
            stream >> method >> path;

            if (path == "/" || path.empty()) {
                return "index.html";  // Default page
            }
            if (path[0] == '/') {
                path.erase(0, 1);  // Remove leading '/'
            }
            return path;
        }

        std::string read_file(const std::string& file_name) {
            std::ifstream file(base_path + "/" + file_name, std::ios::binary);
            if (!file) {
                return "";  // File not found
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        std::string get_mime_type(const std::string& filename) {
            size_t dot_pos = filename.find_last_of(".");
            if (dot_pos != std::string::npos) {
                std::string ext = filename.substr(dot_pos);
                if (mime_types.count(ext)) {
                    return mime_types[ext];
                }
            }
            return "application/octet-stream";  // Default binary type
        }

        int run_server() {
            INIT_SOCKETS();
            int server_fd;
            struct sockaddr_in address;
            int addrlen = sizeof(address);

            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == -1) {
                std::cerr << "Socket creation failed!\n";
                return 1;
            }

            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                std::cerr << "Bind failed on port " << port << "!\n";
                return 1;
            }

            if (listen(server_fd, 3) < 0) {
                std::cerr << "Listen failed!\n";
                return 1;
            }

            std::cout << "Server running on http://localhost:" << port << std::endl;
            server_running = true;

            while (server_running) {
                int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                if (client_socket >= 0) {
                    char buffer[1024] = {0};
                    recv(client_socket, buffer, sizeof(buffer), 0);

                    std::string request(buffer);
                    std::string file_name = get_requested_file(request);
                    std::string file_content = read_file(file_name);

                    std::string status = file_content.empty() ? "404 Not Found" : "200 OK";
                    std::string body = file_content.empty() ? "<h1>404 File Not Found</h1>" : file_content;
                    std::string content_type = get_mime_type(file_name);

                    std::string response = "HTTP/1.1 " + status + "\r\n"
                                        "Content-Type: " + content_type + "\r\n"
                                        "Content-Length: " + std::to_string(body.size()) + "\r\n"
                                        "Connection: close\r\n\r\n" + body;

                    send(client_socket, response.c_str(), response.size(), 0);
                    CLOSE_SOCKET(client_socket);
                }
            }

            std::cout << "Shutting down server on port " << port << "...\n";
            CLOSE_SOCKET(server_fd);
            CLEANUP_SOCKETS();
            return 0;
        }

        static int thread_entry(void* ptr) {
            return static_cast<WebServer*>(ptr)->run_server();
        }

        void start() {
            if (!server_running) {
                server_thread = SDL_CreateThread(thread_entry, "HTTP Server", this);
                if (!server_thread) {
                    std::cerr << "Failed to create server thread! SDL_Error: " << SDL_GetError() << std::endl;
                }
            }
        }
        
        void stop() {
            if (server_running) {
                server_running = false;
                if (server_thread) {
                    SDL_WaitThread(server_thread, nullptr);
                    server_thread = nullptr;
                }
            }
        }
    };
}