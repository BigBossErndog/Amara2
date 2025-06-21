#include <amara2.hpp>

int main(int argc, char** argv) {
    Amara::Creator creator(argc, argv);
    
    std::string indexPath = creator.system.getScriptPath("index");
    if (creator.system.fileExists(indexPath)) {
        creator.startCreation(indexPath);
    }
    return 0;
}