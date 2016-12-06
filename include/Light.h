//
// Created by Niclas Olmenius on 2016-11-02.
//

#pragma once

#include <glm/vec3.hpp>

namespace rengine {

    struct Light {
        glm::vec3 position;
        glm::vec3 color;

        GLfloat linear_factor;
        GLfloat quadratic_factor;

    };

}
