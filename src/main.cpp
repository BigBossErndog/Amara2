#include <amara2.hpp>

int main(int argv, char** args) {
    Amara::Creator creator(argv, args);
    creator.startCreation("index");
    return 0;
}