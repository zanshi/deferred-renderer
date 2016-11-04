//
// Created by Niclas Olmenius on 2016-11-03.
//


#pragma once

#include <GL/glew.h>

namespace rengine {

class Quad {

public:
    Quad();

    void draw() const;

    GLuint vbo_, vao_;

private:


};

}
