//
// Created by Niclas Olmenius on 2016-10-27.
//


#include "Model.h"
#include <algorithm>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <gli/gli.hpp>

namespace rengine {

    void Model::draw(GLuint current_program) const {
        for (auto &&mesh : meshes_) {
            mesh.draw(current_program);
        }
    }

    void Model::load_model(std::string path) {

        // Read file via ASSIMP
        Assimp::Importer importer;
//        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
//                                                       aiProcess_CalcTangentSpace | aiProcess_GenNormals  );

//        const aiScene *scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_Fast);
        const aiScene *scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs |
                                                       aiProcess_PreTransformVertices);
//        const aiScene *scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace);

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

        const bool has_uv = mesh->HasTextureCoords(0);
        const bool has_tangent = mesh->HasTangentsAndBitangents();

        // Walk through each of the mesh's vertices
        for (GLuint i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class
            // that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // Positions
            vertex.position = glm::make_vec3(&mesh->mVertices[i].x);
            vertex.tex_coords = (has_uv) ? glm::make_vec2(&mesh->mTextureCoords[0][i].x) : glm::vec2(0.0f);
            vertex.normal = glm::make_vec3(&mesh->mNormals[i].x);
            vertex.tangent = (has_tangent) ? glm::make_vec3(&mesh->mTangents[i].x) : glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.bitangent = (has_tangent) ? glm::make_vec3(&mesh->mBitangents[i].x) : glm::cross(vertex.normal,
                                                                                                    vertex.tangent);
            vertices.push_back(vertex);
        }
        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (GLuint i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Retrieve all indices of the face and store them in the indices vector
            for (GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Process materials
        if (scene->mNumMaterials > 0) {
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

    std::string get_file_extension(const std::string &filename) {

        if (filename.find_last_of(".") != std::string::npos) {
            return filename.substr(filename.find_last_of(".") + 1);
        } else {
            return "";
        }

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
//                texture.id = TextureFromFile(str.C_Str(), this->directory_);
                texture.id = (get_file_extension(str.C_Str()) == "dds") ? create_texture(str.C_Str()) : TextureFromFile(
                        str.C_Str(), this->directory_);
                texture.type = type_name;
                texture.path = std::string(str.C_Str());
                textures.push_back(texture);
                this->textures_loaded_.push_back(
                        texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }


    /// Filename can be KTX or DDS files
    GLuint Model::create_texture(char const *Filename) {
        std::string path = this->directory_ + std::string(Filename);

        std::cout << path << std::endl;

        gli::texture Texture = gli::load(path);
        if (Texture.empty()) {
            std::cout << "Failed to load " << path << std::endl;
            return 0;
        }


        gli::gl GL(gli::gl::PROFILE_GL33);
        gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
        GLenum Target = GL.translate(Texture.target());

        GLuint TextureName = 0;
        glGenTextures(1, &TextureName);
        glBindTexture(Target, TextureName);
        glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

        glm::tvec3<GLsizei> const Extent(Texture.extent());
        GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

        switch (Texture.target()) {
            case gli::TARGET_1D:
                glTexStorage1D(
                        Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
                break;
            case gli::TARGET_1D_ARRAY:
            case gli::TARGET_2D:
            case gli::TARGET_CUBE:
                glTexStorage2D(
                        Target, static_cast<GLint>(Texture.levels()), Format.Internal,
                        Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
//                glTexImage2D(Target,static_cast<GLint>(Texture.levels()),Format.Internal, Extent.x, Extent.y, 0, Format.External, )
//                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
                break;
            case gli::TARGET_2D_ARRAY:
            case gli::TARGET_3D:
            case gli::TARGET_CUBE_ARRAY:
                glTexStorage3D(
                        Target, static_cast<GLint>(Texture.levels()), Format.Internal,
                        Extent.x, Extent.y,
                        Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
                break;
            default:
                assert(0);
                break;
        }

        for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
            for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
                for (std::size_t Level = 0; Level < Texture.levels(); ++Level) {
                    GLsizei const LayerGL = static_cast<GLsizei>(Layer);
                    glm::tvec3<GLsizei> Extent(Texture.extent(Level));
                    Target = gli::is_target_cube(Texture.target())
                             ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
                             : Target;

                    switch (Texture.target()) {
                        case gli::TARGET_1D:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage1D(
                                        Target, static_cast<GLint>(Level), 0, Extent.x,
                                        Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                        Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage1D(
                                        Target, static_cast<GLint>(Level), 0, Extent.x,
                                        Format.External, Format.Type,
                                        Texture.data(Layer, Face, Level));
                            break;
                        case gli::TARGET_1D_ARRAY:
                        case gli::TARGET_2D:
                        case gli::TARGET_CUBE:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage2D(
                                        Target, static_cast<GLint>(Level),
                                        0, 0,
                                        Extent.x,
                                        Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                                        Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                        Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage2D(
                                        Target, static_cast<GLint>(Level),
                                        0, 0,
                                        Extent.x,
                                        Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                                        Format.External, Format.Type,
                                        Texture.data(Layer, Face, Level));
                            break;
                        case gli::TARGET_2D_ARRAY:
                        case gli::TARGET_3D:
                        case gli::TARGET_CUBE_ARRAY:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage3D(
                                        Target, static_cast<GLint>(Level),
                                        0, 0, 0,
                                        Extent.x, Extent.y,
                                        Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
                                        Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                        Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage3D(
                                        Target, static_cast<GLint>(Level),
                                        0, 0, 0,
                                        Extent.x, Extent.y,
                                        Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
                                        Format.External, Format.Type,
                                        Texture.data(Layer, Face, Level));
                            break;
                        default:
                            assert(0);
                            break;
                    }
                }

        return TextureName;
    }

    GLuint TextureFromFile(const char *path, std::string directory) {
        GLenum format = 3;
        //Generate texture ID and load texture data
        std::string filename = std::string(path);
        std::cout << filename << std::endl;
        filename = directory + '/' + filename;
//        filename = directory + filename;
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

        // TODO implement gamma support, linear color space etc
//        format = GL_RGB;

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





