#include <amara.hpp>
using namespace Amara;
using namespace std;

int main(int argv, char** args) {
    Game game(argv, args);
    game.scripts.run("index.lua");
    return 0;
}