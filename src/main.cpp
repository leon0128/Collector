#include "controller.hpp"

int main(int argc, char **argv)
{
    Controller controller;
    if(controller.initialize())
        controller.run();
    else
        return 1;

    return 0;
}