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

        // Callbacks
        static void error_callback(int error, const char *description);

        static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

        // Keys for GLFW


    private:

        bool init_gl_context();

        bool enable_gl_features();

        bool load_scene();

        void render_scene(GLuint shader_program) const;

        GLFWwindow *window_;

        std::vector<Model> models_;
//        const GLuint nr_lights_ = 64;

        int screen_width_;
        int screen_height_;

        static bool instantiated_;

        bool setup_camera();

        Camera camera_;
        static double last_x;
        static double last_y;
        static bool keys_[1024];
        static bool first_mouse_movement_;

        bool compile_shaders();

        void lighting_pass(const Camera &camera,
                           Shader &g_lighting,
                           const std::vector<glm::vec3> &lightPositions,
                           const std::vector<glm::vec3> &lightColors,
                           const rengine::GBuffer &gbuffer) const;

        void geometry_pass(Shader &g_geometry_shader, GBuffer &gbuffer) const;

        void handle_input(float delta_time);

        void update_camera(GLuint ubo_transforms);

        void update_lights(std::vector<glm::vec3> &light_positions, GLfloat time);


        void update_window_title(const GLfloat time) const;
    };

}