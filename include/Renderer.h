//
// Created by Niclas Olmenius on 2016-10-28.
//


#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include "Mesh.h"
#include "Model.h"

namespace rengine {

class Renderer {

public:
    Renderer();
    ~Renderer();

    void start_up();
    void shut_down();

    void run();

private:
    bool instantiated_;


};


}
