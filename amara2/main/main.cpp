#include <amara2.hpp>

int main(int argc, char** argv) {
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