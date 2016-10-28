//
// Created by Niclas Olmenius on 2016-10-27.
//

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace rengine {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

}