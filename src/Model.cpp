//
// Created by Niclas Olmenius on 2016-10-27.
//


#include "Model.h"
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

namespace rengine {

    void Model::draw(GLuint current_program) const {

        for (auto &&mesh : meshes_) {
            mesh.draw(current_program);
        }

    }

    void Model::load_model(std::string path) {

        // Read file via ASSIMP
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                       aiProcess_CalcTangentSpace);

//        const aiScene *scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_Fast);
//        const aiScene *scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

        // Check for errors
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // Retrieve the directory path of the filepath
        this->directory_ = path.substr(0, path.find_last_of('/'));

        // Process ASSIMP's root node recursively
        this->process_node(scene->mRootNode, scene);
    }

    void Model::process_node(aiNode *node, const aiScene *scene) {
        // Process each mesh located at the current node
        for (GLuint i = 0; i < node->mNumMeshes; i++) {
            // The node object only contains indices to index the actual objects in the scene.
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes_.push_back(this->process_mesh(mesh, scene));
        }
        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (GLuint i = 0; i < node->mNumChildren; i++) {
            this->process_node(node->mChildren[i], scene);
        }
    }

    Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
        // Data to fill
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;

        // Walk through each of the mesh's vertices
        for (GLuint i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3
                    vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

//            std::cout << vector.x << " " << vector.y << " " << vector.z << std::endl;

            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            if(mesh->mTangents) {
                // Tangents
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;

//            // Bitangents
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }
//

            // Texture Coordinates
            if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.tex_coords = vec;
            } else
                vertex.tex_coords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (GLuint i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Retrieve all indices of the face and store them in the indices vector
            for (GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

//        if(scene->HasTextures())


        // Process materials
        if (scene->mNumMaterials != 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
            // Same applies to other texture as the following list summarizes:
            // Diffuse: texture_diffuseN
            // Specular: texture_specularN
            // Normal: texture_normalN

            // 1. Diffuse maps
            std::vector<Texture> diffuse_maps = this->load_material_textures(material, aiTextureType_DIFFUSE,
                                                                             "texture_diffuse");
            textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
            // 2. Specular maps
            std::vector<Texture> specular_maps = this->load_material_textures(material, aiTextureType_SPECULAR,
                                                                              "texture_specular");
            textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

            std::vector<Texture> normal_maps = load_material_textures(material, aiTextureType_NORMALS,
                                                                      "texture_normal");
            textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
        }

        // Return a mesh object created from the extracted mesh data

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> Model::load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name) {
        std::vector<Texture> textures;
        for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            GLboolean skip = GL_FALSE;
            for (GLuint j = 0; j < textures_loaded_.size(); j++) {
                if (textures_loaded_[j].path == std::string(str.C_Str())) {
                    textures.push_back(textures_loaded_[j]);
                    skip =
                            GL_TRUE; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip) {   // If texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory_);
                texture.type = type_name;
                texture.path = std::string(str.C_Str());
                textures.push_back(texture);
                this->textures_loaded_.push_back(
                        texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    GLuint TextureFromFile(const char *path, std::string directory) {
        GLenum format = 3;
        //Generate texture ID and load texture data
        std::string filename = std::string(path);
        filename = directory + '/' + filename;
        GLuint textureID;
        glGenTextures(1, &textureID);
        int width, height, channels;
        unsigned char *image = stbi_load(filename.c_str(), &width, &height, &channels, 0);
        if (!image) {
            fprintf(stderr, "%s %s\n", "Failed to Load Texture", filename.c_str());
        }
        assert(image != nullptr);

        // Set the Correct Channel Format
        switch (channels) {
            case 1 :
                format = GL_ALPHA;
                break;
            case 2 :
                format = GL_LUMINANCE;
                break;
            case 3 :
                format = GL_RGB;
                break;
            case 4 :
                format = GL_RGBA;
                break;
            default:
                break;
        }

        // Assign texture to ID
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);

        return textureID;
    }
}





