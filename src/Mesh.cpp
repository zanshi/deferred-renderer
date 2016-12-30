//
// Created by Niclas Olmenius
//

#include "Mesh.h"

namespace rengine {

    Mesh::Mesh(std::vector<Vertex> vertices,
               std::vector<GLuint> indices,
               std::vector<Texture> textures)
            : textures_{textures} {
        indices_size_ = indices.size();

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);

        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                     &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (GLvoid *) 0);
        // Vertex Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, tex_coords));
        // Vertex Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, tangent));
        // Vertex Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, bitangent));

        glBindVertexArray(0);

    }


    void Mesh::draw() const {

        // Right now I assume that every mesh has a diffuse, specular and normal texture
        // diffuse idx = 0
        // specular idx = 1
        // normal idx = 2
        for (GLuint i = 0; i < textures_.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
            glBindTexture(GL_TEXTURE_2D, textures_[i].id);
        }

        // Draw mesh
        // TODO Use packet buffers in the future
        glBindVertexArray(VAO_);
        glDrawElements(GL_TRIANGLES, indices_size_, GL_UNSIGNED_INT, 0);


        // Always good practice to set everything back to defaults once configured.
        for (GLuint i = 0; i < textures_.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    }

}