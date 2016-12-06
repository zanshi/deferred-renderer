//
// Created by niclas on 2016-10-21.
//

#include "Engine.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <stdlib.h>
#include <DeferredRenderer.h>
#include <Light.h>

namespace rengine {

    bool Engine::instantiated_ = false;

    double Engine::last_x;
    double Engine::last_y;
    bool Engine::keys_[1024];
    bool Engine::first_mouse_movement_;
    GLuint Engine::show_normals_;

    Engine::Engine(int width, int height)
            : window_width_{width}, window_height_{height} {
        assert(!instantiated_); // prevent more than one instance from being started
        instantiated_ = true;
    }

    Engine::~Engine() {
        instantiated_ = false;
    }

    void Engine::start_up() {

        if (!init_gl_context())
            exit(EXIT_FAILURE);

        if (!enable_gl_features())
            exit(EXIT_FAILURE);

        if (!load_scene())
            exit(EXIT_FAILURE);

        if (!setup_camera())
            exit(EXIT_FAILURE);

        if (!compile_shaders())
            exit(EXIT_FAILURE);

    }

    void Engine::shut_down() {
        glfwDestroyWindow(window_);
        glfwTerminate();

    }

    bool Engine::init_gl_context() {

        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            return false;
//        exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        window_ = glfwCreateWindow(window_width_, window_height_, "Deferred renderer", NULL, NULL);
        if (!window_) {
            glfwTerminate();
            return false;
//        exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window_);

        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, key_callback);
        glfwSetCursorPosCallback(window_, mouse_callback);

        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cout << "Failed to initialize GLEW" << std::endl;
            return -1;
        }

        printf("GL version:      %s\n", glGetString(GL_VERSION));

        glfwGetFramebufferSize(window_, &window_width_, &window_height_);
        glViewport(0, 0, window_width_, window_height_);


        show_normals_ = 1;

        return true;

    }

    bool Engine::enable_gl_features() {

//        glEnable(GL_DEPTH_TEST);

        return true;
    }

    bool Engine::load_scene() {

//        models_.push_back(Model{"../assets/models/nano/nanosuit.obj"});
//        models_.push_back(Model{"../assets/bible/objects/dragon.sbm"});
//        models_.push_back(Model{"../assets/models/blender/Blenderman.dae"});
//        models_.push_back(Model{"../assets/models/e.dae"});
//        models_.push_back(Model{"../assets/models/bmw/bmw.obj"});
//        models_.push_back(Model{"../assets/models/cottage/Snow covered CottageFBX.fbx"});
//        models_.push_back(Model{"../assets/models/bmx/bmx.obj"});
//        models_.push_back(Model{"../assets/models/city/Scifi Floating City/Scifi Floating City.obj"});
//        models_.push_back(Model{"../assets/models/bathroom/BathroomSet02.obj"});
//        models_.push_back(Model{"../assets/models/rungholt/rungholt.obj"});
//        models_.push_back(Model{"../assets/models/crytek/sponza.obj"});
//        models_.push_back(Model{"../assets/models/cyborg/Cyborg.obj"});
//        models_.push_back(Model{"../assets/models/head/head.OBJ"});
        models_.push_back(Model{"../assets/models/sponza2/sponza.dae"});
//        models_.push_back(Model{"../assets/models/crytek-sponza/sponza.obj"});

        return true;

    }


    bool Engine::setup_camera() {
        camera_ = Camera{glm::vec3(0.0f, 0.0f, 4.0f)};
        first_mouse_movement_ = true;

        return true;
    }

    bool Engine::compile_shaders() {

        return true;
    }

    void Engine::run() {

        // Create and compile shaders
        auto geometry_shader = Shader{"../assets/shaders/geometry.vert", "../assets/shaders/geometry.frag"};
        auto lighting_shader = Shader{"../assets/shaders/hdr_lighting.vert", "../assets/shaders/hdr_lighting.frag"};
        auto filter_shader = Shader{"../assets/shaders/gaussian_blur.vert", "../assets/shaders/gaussian_blur.frag"};
        auto combine_bloom_shader = Shader{"../assets/shaders/bloom_combine.vert",
                                           "../assets/shaders/bloom_combine.frag"};


        // Set up lights
        // - Colors
        std::vector<glm::vec3> light_positions;
        std::vector<glm::vec3> light_colors;

        std::vector<Light> lights;

        setup_lights(lights);

        const auto deferred_renderer = DeferredRenderer{window_width_, window_height_, geometry_shader,
                                                        lighting_shader};

        // ------------------------------------------------------------------------------------
        // Set up framebuffers
        const auto render_fbo = FBO{window_width_, window_height_, 2, true};
        const auto filter_fbos = std::array<FBO, 2>{{FBO{window_height_, window_width_, 1, false},
                                                            FBO{window_width_, window_height_, 1, false}}};

        // Create quad for rendering the final image
        auto quad = Quad{};

        // ------------------------------------------------------------------------------------

        GLfloat current_frame_time = 0.0f;
        GLfloat delta_time = 0.0f;
        GLfloat last_frame_time = 0.0f;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        while (!glfwWindowShouldClose(window_)) {

            // ------------------------------------------------------
            // Delta time calculations
            current_frame_time = static_cast<float>(glfwGetTime());
            delta_time = current_frame_time - last_frame_time;
            last_frame_time = current_frame_time;

            update_window_title(current_frame_time);

            // ------------------------------------------------------

            // INPUT
            glfwPollEvents();
            handle_input(delta_time);

            // LIGHTS
            update_lights(lights);

            // ------------------------------------------------------
            // RENDERING



            bloom_pass(render_fbo, filter_fbos, filter_shader, combine_bloom_shader, quad);


            // Swap the screen buffers
            glfwSwapBuffers(window_);
        }

    }

    void Engine::setup_lights(std::vector<Light> &lights) {
        const GLuint NR_LIGHTS = 32;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);

        light_linear_factor_ = 0.35;
        light_quadratic_factor_ = 0.44;

        for (GLuint i = 0; i < NR_LIGHTS; i++) {
            // Calculate slightly random offsets
//            GLfloat xPos = static_cast<GLfloat>(dis(gen) * 6.0f - 3.0f);
//            GLfloat yPos = static_cast<GLfloat>(dis(gen) * 6.0f - 3.0f + 4.0f);
//            GLfloat zPos = static_cast<GLfloat>(dis(gen) * 6.0f - 3.0f);
//            GLfloat xPos = 0.1f * i * 6.0f - 3.0f;
            GLfloat xPos = 0.0;
            GLfloat yPos = 0.1f * i * 6.0f - 3.0f;
            GLfloat zPos = 0.1f * i * 6.0f - 3.0f;

            glm::vec3 pos{xPos, yPos, zPos};
            pos *= 10.0f;

            // Also calculate random color
            GLfloat rColor = static_cast<GLfloat>(dis(gen) / 2.0f + 0.5f); // Between 0.5 and 1.0
            GLfloat gColor = static_cast<GLfloat>(dis(gen) / 2.0f + 0.5f); // Between 0.5 and 1.0
            GLfloat bColor = static_cast<GLfloat>(dis(gen) / 2.0f + 0.5f); // Between 0.5 and 1.0
//            GLfloat rColor = 1.0f - 0.01f * i; // Between 0.5 and 1.0
//            GLfloat gColor = 1.0f - 0.02f * i; // Between 0.5 and 1.0
//            GLfloat bColor = 1.0f - 0.03f * i; // Between 0.5 and 1.0
//            std::cout << rColor << std::endl;

            Light temp_light;
            temp_light.position = pos;
            temp_light.color = {rColor, gColor, bColor};

            temp_light.linear_factor = light_linear_factor_;
            temp_light.quadratic_factor = light_quadratic_factor_;

            lights.push_back(temp_light);
        }
    }

    void Engine::bloom_pass(const FBO &render_fbo, const std::array<FBO, 2> &filter_fbos, const Shader &shader_filter,
                            const Shader &shader_combine, const Quad &quad) const {


        //-----------------------------------------------
        // Filter

        shader_filter.use();
        glUniform1i(glGetUniformLocation(shader_filter.program_, "hdr_image"), 0);

        glBindVertexArray(quad.vao_); // Bind the quad's VAO

        filter_fbos[0].bind_draw(); // Use the first filter FBO

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo.textures_[1]); // Bind the brightpass texture

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        filter_fbos[1].bind_draw(); // Use the second filter FBO

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, filter_fbos[0].textures_[0]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);



        //-----------------------------------------------
        // Combine

        shader_combine.use();
        glUniform1i(glGetUniformLocation(shader_combine.program_, "hdr_image"), 0);
        glUniform1i(glGetUniformLocation(shader_combine.program_, "bloom_image"), 1);

        // Render to default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, filter_fbos[1].textures_[0]); // Use the texture in the second filter FBO

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo.textures_[0]); // And the original HDR image

        // Finally draw the result

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    void Engine::render_scene(GLuint shader_program) const {
        for (auto &&model : models_) {
            model.draw(shader_program);
        }
    }


    void Engine::handle_input(float delta_time) {

        // Camera controls
        if (keys_[GLFW_KEY_W])
            camera_.ProcessKeyboard(FORWARD, delta_time);
        if (keys_[GLFW_KEY_S])
            camera_.ProcessKeyboard(BACKWARD, delta_time);
        if (keys_[GLFW_KEY_A])
            camera_.ProcessKeyboard(LEFT, delta_time);
        if (keys_[GLFW_KEY_D])
            camera_.ProcessKeyboard(RIGHT, delta_time);
        if (keys_[GLFW_KEY_1]) {
            light_linear_factor_ = 0.7f;
            light_quadratic_factor_ = 1.8f;
        }
        if (keys_[GLFW_KEY_2]) {
            light_linear_factor_ = 0.35f;
            light_quadratic_factor_ = 0.44f;
        }
        if (keys_[GLFW_KEY_3]) {
            light_linear_factor_ = 0.14f;
            light_quadratic_factor_ = 0.07f;
        }
        if (keys_[GLFW_KEY_4]) {
            light_linear_factor_ = 0.045f;
            light_quadratic_factor_ = 0.0075f;
        }
        if (keys_[GLFW_KEY_5]) {
            light_linear_factor_ = 0.022f;
            light_quadratic_factor_ = 0.0019f;
        }
        if (keys_[GLFW_KEY_6]) {
            light_linear_factor_ = 0.012f;
            light_quadratic_factor_ = 0.0007f;
        }

        if (keys_[GLFW_KEY_SPACE]) {
            show_normals_ = (show_normals_ == 1) ? 0 : 1;
        }


    }

    void Engine::update_camera(GLuint ubo_transforms) const {

        // Set up the projection matrix and feed the data to the uniform block object
        glm::mat4 projection = glm::perspective(camera_.Zoom, (GLfloat) window_width_ / (GLfloat) window_height_, 0.1f,
                                                1000.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
//        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // To the same for the view matrix
        glm::mat4 view = camera_.GetViewMatrix();
//        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferSubData(
                GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }


    void Engine::update_lights(std::vector<Light> &lights) {

        for (auto &&light : lights) {
            light.linear_factor = light_linear_factor_;
            light.quadratic_factor = light_quadratic_factor_;
        }

    }

    void Engine::error_callback(int error, const char *description) {
        fputs(description, stderr);
    }

    void Engine::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (key >= 0 && key < 1024) {
            if (action == GLFW_PRESS)
                keys_[key] = true;
            else if (action == GLFW_RELEASE)
                keys_[key] = false;
        }
    }

    void Engine::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
        Engine *engine = static_cast<Engine *>(glfwGetWindowUserPointer(window));

        if (first_mouse_movement_) {
            last_x = xpos;
            last_y = ypos;
            first_mouse_movement_ = false;
        }

        GLfloat xoffset = static_cast<GLfloat>(xpos - last_x);
        GLfloat yoffset = static_cast<GLfloat>(last_y - ypos);

        last_x = xpos;
        last_y = ypos;

        engine->camera_.ProcessMouseMovement(xoffset, yoffset);
    }

    void Engine::update_window_title(const GLfloat t) const {

        static double t0 = 0.0;
        static int frames = 0;
        static double fps = 0.0;
        static double frametime = 0.0;
        static char titlestring[200];

        // If one second has passed, or if this is the very first frame
        if ((t - t0) > 1.0 || frames == 0) {
            fps = (double) frames / (t - t0);
            if (frames > 0) frametime = 1000.0 * (t - t0) / frames;
            sprintf(titlestring, "Deferred renderer, %.2f ms/frame (%.1f FPS)", frametime, fps);
            glfwSetWindowTitle(window_, titlestring);
            // printf("Speed: %.1f FPS\n", fps);
            t0 = t;
            frames = 0;
        }
        frames++;

    }


}