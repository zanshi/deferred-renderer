//
// Created by Niclas Olmenius on 2016-10-27.
//


#pragma once

#include <assimp/material.h>
#include <assimp/scene.h>
#include "Mesh.h"

namespace rengine {

    GLuint TextureFromFile(const char* path, std::string directory);
    std::string get_file_extension(const std::string& filename);

    class Model {
    public:
        /*  Functions   */
        Model(std::string path)
        {
            this->load_model(path);
        }

        void draw() const;

    private:
        /*  Model Data  */
        std::vector<Mesh> meshes_;
        std::string directory_;
        std::vector<Texture> textures_loaded_;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.


        /*  Functions   */
        void load_model(std::string path);

        void process_node(aiNode* node, const aiScene* scene);

        Mesh process_mesh(aiMesh* mesh, const aiScene* scene);

        std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type,
                std::string type_name);


        GLuint create_texture(char const* Filename); // From gli example

    };

}


