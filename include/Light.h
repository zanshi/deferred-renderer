//
// Created by Niclas Olmenius on 2016-11-02.
//

#pragma once

#include <glm/vec3.hpp>
#include <GL/glew.h>

namespace rengine {

#pragma pack (push, 1)
    struct Light {
        glm::vec3 position;
        unsigned int        : 32;       // pad0
        glm::vec3 color;
        unsigned int        : 32;       // pad1
        GLfloat linear;
        GLfloat quadratic;
        GLfloat radius;                 // pad2
        unsigned int        : 32;       // pad3
    };
#pragma pack (pop)

}
