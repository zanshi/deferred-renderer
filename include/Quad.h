//
// Created by Niclas Olmenius
//


#pragma once

#include <GL/glew.h>

namespace rengine {

class Quad {

public:
    Quad();

    void init();
    void draw() const;

    GLuint vbo_, vao_;

};

}
