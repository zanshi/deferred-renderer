//
// Created by Niclas Olmenius on 2016-10-27.
//

#include "Mesh.h"

namespace rengine {

    Mesh::Mesh(std::vector<Vertex> vertices,
            std::vector<GLuint> indices,
            std::vector<Texture> textures)
            :vertices_{vertices},
             indices_{indices},
             textures_{textures}
    {
        setup_mesh();
    }

    void Mesh::draw(GLuint current_program) const
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        for(GLuint i = 0; i < this->textures_.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
            // Retrieve texture number (the N in diffuse_textureN)
            std::stringstream ss;
            std::string number;
            std::string name = this->textures_[i].type;
            if(name == "texture_diffuse")
                ss << diffuseNr++; // Transfer GLuint to stream
            else if(name == "texture_specular")
                ss << specularNr++; // Transfer GLuint to stream
            number = ss.str();

            glUniform1f(glGetUniformLocation(current_program, ("material." + name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures_[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // Draw mesh
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indices_.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


    }

    void Mesh::setup_mesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        glBufferData(GL_ARRAY_BUFFER, this->vertices_.size()*sizeof(Vertex),
                &this->vertices_[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices_.size()*sizeof(GLuint),
                &this->indices_[0], GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                (GLvoid*) 0);
        // Vertex Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                (GLvoid*) offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                (GLvoid*) offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

    }

}