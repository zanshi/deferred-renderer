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
        std::vector<Vertex> vertices_;
        std::vector<GLuint> indices_;
        std::vector<Texture> textures_;

        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

        void draw(GLuint current_program) const;

    private:
        /*  Render data  */
        GLuint VAO, VBO, EBO;

        /*  Functions    */
        void setup_mesh();

    };

}




