#include <amara2.hpp>

#if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
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

int main(int argc, char** argv) {
    #if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
    EnsureStandardHandles();
    #endif
    
    Amara::Creator creator(argc, argv);
    
    std::string indexPath = creator.system.getScriptPath("index");
    if (creator.system.exists(indexPath)) {
        return creator.startCreation(indexPath);
    }
    return 1;
}

#if defined(_WIN32) && !defined(AMARA_DEBUG_BUILD)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main(__argc, __argv);
}
#endif