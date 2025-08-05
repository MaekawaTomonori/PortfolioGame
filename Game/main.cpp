#include <memory>
#include "MyGame.hpp"
#include "include/Framework.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){

    Framework project;
    project.Execute(std::make_unique<MyGame>());

    return 0;
}

