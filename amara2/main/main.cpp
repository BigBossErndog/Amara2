#if defined(__ANDROID__)
#define SDL_MAIN_HANDLED
#endif

#include <amara2.hpp>

#if defined(__ANDROID__)
#include <SDL3/SDL_main.h>
#endif

#if defined(_WIN32)
void EnsureStandardHandles() {
    OutputDebugStringA("Amara Engine: Checking standard handles...\n");

    HANDLE std_handles[] = { GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE) };
    const char* handle_names[] = { "STDIN", "STDOUT", "STDERR" };
    DWORD std_types[] = { STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
    DWORD access_types[] = { GENERIC_READ, GENERIC_WRITE, GENERIC_WRITE };

    for (int i = 0; i < 3; ++i) {
        if (std_handles[i] == NULL || std_handles[i] == INVALID_HANDLE_VALUE) {
            char log_buffer[256];
            snprintf(log_buffer, sizeof(log_buffer), "Amara Engine: %s handle is invalid. Redirecting to NUL.\n", handle_names[i]);
            OutputDebugStringA(log_buffer);

            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;

            HANDLE nul_handle = CreateFileW(L"NUL", access_types[i], FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, NULL);
            if (nul_handle != INVALID_HANDLE_VALUE) {
                if (!SetStdHandle(std_types[i], nul_handle)) {
                     OutputDebugStringA("Amara Engine: SetStdHandle failed.\n");
                }
            } else {
                OutputDebugStringA("Amara Engine: Failed to open NUL device for standard handle redirection.\n");
            }
        }
    }
}
#endif

#if defined(__ANDROID__)
extern "C" int SDL_main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
    #if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
    EnsureStandardHandles();
    #endif
#endif
    std::string build_args_str;
    #if defined(AMARA_BUILD_ARGS)
        build_args_str = AMARA_BUILD_ARGS;
    #endif
    #if defined(AMARA_ENCRYPTION_KEY)
        #if defined(AMARA_ENCRYPT_OUTPUT)
            return amara_run(argc, argv, AMARA_STRINGIFY(AMARA_ENCRYPTION_KEY), true, build_args_str);
        #else
            return amara_run(argc, argv, AMARA_STRINGIFY(AMARA_ENCRYPTION_KEY), false, build_args_str);
        #endif
    #else
        return amara_run(argc, argv, "", false, "");
    #endif
}

#if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main(__argc, __argv);
}
#endif