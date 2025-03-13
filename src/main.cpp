#include <amara.hpp>
using namespace Amara;
using namespace std;

int main(int argv, char** args) {
    Game game;
    vector<string> contents = game.files.getDirectoryContents("../");
    for (auto it: contents) {
        cout << it << endl;
    }
    // sol::state lua;
    // lua.open_libraries(sol::lib::base);
    
    // lua.script(R"(
    //     print("hello")
    // )");
    return 0;
}