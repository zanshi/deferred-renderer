//
// Created by Niclas Olmenius
//

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <array>
#include "Mesh.h"
#include "Model.h"
#include "camera.h"
#include "GBuffer.h"
#include "FBO.h"
#include "Quad.h"
#include "Light.h"

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


    private:

        static bool instantiated_;
        static double last_x;
        static double last_y;
        static bool keys_[1024];
        static bool first_mouse_movement_;

        bool init_gl_context();

        bool enable_gl_features();

        bool load_scene();

        void render_scene() const;

        GLFWwindow *window_;

        int window_width_;
        int window_height_;

        std::vector<Model> models_;

        Camera camera_;
        bool camera_enabled_;

        bool setup_camera();

        GLuint ubo_transforms_;

        // Lights
        std::vector<Light> lights_;
        Light *lights_gl_;
        GLfloat light_linear_factor_;
        GLfloat light_quadratic_factor_;
        GLuint lights_ubo_;
        const unsigned int nrOfLights_ = 32;

        static GLuint show_normals_;
        static bool should_render_deferred_;

        Quad quad_;

        bool compile_shaders();

        void render_forward(const Shader &forward_shader, const FBO &render_fbo) const;


        void render_deferred(const Shader &geometry_shader, const Shader &lighting_shader, const FBO &render_fbo,
                             const GBuffer &gbuffer) const;

        void deferred_lighting_pass(const Shader &lighting_shader, const GBuffer &gbuffer, const FBO &render_fbo) const;

        void deferred_geometry_pass(const Shader &g_geometry_shader, const GBuffer &gbuffer) const;

        void bloom_pass(const FBO &render_fbo, const std::array<FBO, 2> &filter_fbos, const Shader &shader_filter,
                        const Shader &shader_combine) const;

        void handle_input(float delta_time);

        void update_camera(const glm::mat4 &model) const;

        void update_lights(const Shader &shader) const;

        void update_window_title(const GLfloat time) const;

        void setup_lights();


    };

}