//
// Created by Niclas Olmenius on 2016-10-27.
//

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace rengine {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 tex_coords;
    };

}