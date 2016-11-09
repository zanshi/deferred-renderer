//
// Created by niclas on 2016-10-21.
//

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include "Mesh.h"
#include "Model.h"
#include "camera.h"
#include "GBuffer.h"

namespace rengine {

    class Engine {

    public:

        Engine(int width, int height);

        ~Engine();

        void run();

        void start_up();

        void shut_down();

        // Keys for GLFW
        bool keys_[1024];

    private:

        bool init_gl_context();

        bool enable_gl_features();

        bool load_scene();

        void render_scene(GLuint shader_program) const;


        GLFWwindow *window_;

        const aiScene *scene_;
        std::vector<Mesh> meshes_;
        std::vector<Model> models_;
        GLuint current_program_;
        const GLuint nr_lights_ = 64;

        int screen_width_;
        int screen_height_;

        static bool instantiated_;

        bool setup_camera();
        Camera camera_;


        bool compile_shaders();

        void lighting_pass(const Camera &camera,
                           Shader &g_lighting,
                           const std::vector<glm::vec3> &lightPositions,
                           const std::vector<glm::vec3> &lightColors,
                           const rengine::GBuffer &gbuffer) const;

        void geometry_pass(Shader &g_geometry_shader, GBuffer &gbuffer) const;

        void handle_input(float delta_time);

        void update_camera(GLuint ubo_transforms);
    };

}