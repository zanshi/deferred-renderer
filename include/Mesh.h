//
// Created by Niclas Olmenius on 2016-10-27.
//


#pragma once

#include <vector>
#include "Shader.h"
#include "Vertex.h"
#include "Texture.h"

namespace rengine {

    class Mesh {

    public:
        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
        void draw(GLuint current_program) const;

    private:
        /*  Render data  */
        std::vector<Texture> textures_;
        GLuint VAO_, VBO_, EBO_;
        GLuint indices_size_;

    };

}




