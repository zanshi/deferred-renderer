//
// Created by Niclas Olmenius
//

#include "Engine.h"

const int default_width = 1280;
const int default_height = 800;

int main(int argc, char *argv[]) {

    int width = default_width;
    int height = default_height;

    // Read width and height from standard input
    if (argc > 2) {
        width = std::stoi(argv[1]);
        height = std::stoi(argv[2]);
    }

    rengine::Engine engine(width, height);

    engine.start_up();
    engine.run();
    engine.shut_down();

    return 0;

}
