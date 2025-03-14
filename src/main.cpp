#include <amara.hpp>
using namespace Amara;
using namespace std;

int main(int argv, char** args) {
    Game game;
    // Entity entity;
    // game.lua["entity"] = &entity;
    // game.files.luaGetDirectoryContents("./");
    game.run("index.lua");
    return 0;
}