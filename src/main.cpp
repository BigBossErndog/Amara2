#include <amara2.hpp>

#if defined(_WIN32)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i) {
        int len = wcslen(argvW[i]) + 1;
        argv[i] = new char[len];
        wcstombs(argv[i], argvW[i], len);
    }

    Amara::Creator creator(argc, argv);
    creator.startCreation("index");

    for (int i = 0; i < argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
    LocalFree(argvW);
}
#else
int main(int argc, char** argv) {
    Amara::Creator creator(argc, argv);
    creator.startCreation("index");
    return 0;
}
#endif