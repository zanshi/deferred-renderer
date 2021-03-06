//
// Created by Niclas Olmenius
//

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace rengine {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coords;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

}