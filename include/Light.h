//
// Created by Niclas Olmenius
//

#pragma once

#include <glm/vec3.hpp>
#include <GL/glew.h>

namespace rengine {

#pragma pack (push, 1)
    // The struct is padded to be able to use the std140 layout
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
