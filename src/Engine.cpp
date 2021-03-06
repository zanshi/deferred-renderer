//
// Created by Niclas Olmenius
//

#include "Engine.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

namespace rengine {

    bool Engine::instantiated_ = false;

    double Engine::last_x;
    double Engine::last_y;
    bool Engine::keys_[1024];
    bool Engine::first_mouse_movement_;
    GLuint Engine::show_normals_;
    bool Engine::should_render_deferred_;

    Engine::Engine(int width, int height)
            : window_width_{width}, window_height_{height} {
        assert(!instantiated_); // prevent more than one instance from being started
        instantiated_ = true;
    }

    Engine::~Engine() {
        instantiated_ = false;
    }

    void Engine::start_up() {

        should_render_deferred_ = false;
        show_normals_ = 0;

        bool success = init_gl_context();
        assert(success);

        success = load_scene();
        assert(success);

        success = setup_camera();
        assert(success);

    }

    void Engine::shut_down() {
        glfwDestroyWindow(window_);
        glfwTerminate();

    }

    bool Engine::init_gl_context() {

        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        window_ = glfwCreateWindow(window_width_, window_height_, "Deferred renderer", NULL, NULL);
        if (!window_) {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);

        glfwSetWindowUserPointer(window_, this);
        glfwSetKeyCallback(window_, key_callback);
        glfwSetCursorPosCallback(window_, mouse_callback);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cout << "Failed to initialize GLEW" << std::endl;
            return -1;
        }

        printf("GL version:      %s\n", glGetString(GL_VERSION));

        ImGui_ImplGlfwGL3_Init(window_, true);

        glfwGetFramebufferSize(window_, &window_width_, &window_height_);
        glViewport(0, 0, window_width_, window_height_);


        return true;

    }

    bool Engine::load_scene() {

        models_.push_back(Model{"../assets/models/sponza.dae"});

        return true;

    }


    bool Engine::setup_camera() {
        // Position the camera at a good location for testing
        camera_ = Camera{glm::vec3(-120.0f, 20.0f, 0.0f)};
        first_mouse_movement_ = true;
        camera_enabled_ = false;

        return true;
    }


    void Engine::run() {

        // Create and compile common shaders
        const auto filter_shader = Shader{"../assets/shaders/gaussian_blur.vert",
                                          "../assets/shaders/gaussian_blur.frag"};
        const auto combine_bloom_shader = Shader{"../assets/shaders/bloom_combine.vert",
                                                 "../assets/shaders/bloom_combine.frag"};

        // Forward shader
        const auto forward_shader = Shader{"../assets/shaders/forward.vert",
                                           "../assets/shaders/forward.frag"};

        // ------------------------------------------------------------------------------------
        // Deferred shaders
        const auto deferred_geometry_shader = Shader{"../assets/shaders/deferred_geometry.vert",
                                                     "../assets/shaders/deferred_geometry.frag"};
        const auto deferred_lighting_shader = Shader{"../assets/shaders/deferred_hdr_lighting.vert",
                                                     "../assets/shaders/deferred_hdr_lighting.frag"};

        deferred_geometry_shader.use();

        // ------------------------------------------------------------------------------------
        // Set up the uniform transform block
        glGenBuffers(1, &ubo_transforms_);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms_);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        // ------------------------------------------------------------------------------------
        // Set up lights
        glGenBuffers(1, &lights_ubo_);

        glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);
        glBufferData(GL_UNIFORM_BUFFER, nrOfLights_ * sizeof(Light), nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        setup_lights();

        // ------------------------------------------------------------------------------------
        // Set up frame buffers
        const auto gbuffer = GBuffer{window_width_, window_height_};
        const auto render_fbo = FBO{window_width_, window_height_, 2, true};
        const auto filter_fbos = std::array<FBO, 2>{{FBO{window_height_, window_width_},
                                                            FBO{window_width_, window_height_}}};

        // Create quad for rendering the final image
        quad_.init();

        // ------------------------------------------------------------------------------------
        GLfloat current_frame_time = 0.0f;
        GLfloat delta_time = 0.0f;
        GLfloat last_frame_time = 0.0f;

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

            ImGui_ImplGlfwGL3_NewFrame();

            {
                handle_input(delta_time);

                auto str = should_render_deferred_ ? "Using deferred shading" : "Using forward shading";
                ImGui::Text(str);
                ImGui::SliderFloat("Linear light factor", &light_linear_factor_, 0.0f, 2.0f);
                ImGui::SliderFloat("Quadratic light factor", &light_quadratic_factor_, 0.0f, 0.1f);
                if (ImGui::Button("Toggle render deferred")) should_render_deferred_ ^= 1;
            }


            // LIGHTS
            update_lights(forward_shader);

            // ------------------------------------------------------
            // RENDERING
            if (should_render_deferred_) {
                // Deferred
                render_deferred(deferred_geometry_shader, deferred_lighting_shader, render_fbo, gbuffer);
            } else {
                // Forward
                render_forward(forward_shader, render_fbo);
            }

            bloom_pass(render_fbo, filter_fbos, filter_shader, combine_bloom_shader);

            // GUI
            ImGui::Render();

            // Swap the screen buffers
            glfwSwapBuffers(window_);
        }

        ImGui_ImplGlfwGL3_Shutdown();

    }

    void
    Engine::render_deferred(const Shader &geometry_shader, const Shader &lighting_shader, const FBO &render_fbo,
                            const GBuffer &gbuffer) const {

        // Geometry pass
        deferred_geometry_pass(geometry_shader, gbuffer);
        // Lighting pass
        deferred_lighting_pass(lighting_shader, gbuffer, render_fbo);

    }

    void
    Engine::render_forward(const Shader &forward_shader, const FBO &render_fbo) const {

        // Bind the render FBO for drawing
        render_fbo.bind();

        // Clear the FBO so that we have a clean frame
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat float_ones = 1.0f;
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_COLOR, 1, black);
        glClearBufferfv(GL_DEPTH, 0, &float_ones);

        forward_shader.use();

        glUniform1ui(glGetUniformLocation(forward_shader.program_, "showNormals"), show_normals_);
        glUniform3fv(glGetUniformLocation(forward_shader.program_, "viewPos"), 1, glm::value_ptr(camera_.Position));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        const glm::mat4 model;

        // CAMERA
        update_camera(model);

        // Render the scene. (Also binds relevant textures)
        render_scene();

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

    }


    void Engine::setup_lights() {

        light_linear_factor_ = 0.14f;
        light_quadratic_factor_ = 0.07f;

    }


    void Engine::deferred_geometry_pass(const Shader &g_geometry_shader, const GBuffer &gbuffer) const {

        gbuffer.bind_for_geometry_pass();

        static const GLuint uint_zeros[] = {0, 0, 0, 0};
        static const GLfloat float_zeros[] = {0.0f, 0.0f, 0.0f, 0.0f};
        static const GLfloat float_ones[] = {1.0f, 1.0f, 1.0f, 1.0f};

        glClearBufferuiv(GL_COLOR, 0, uint_zeros);
        glClearBufferfv(GL_COLOR, 1, float_zeros);
        glClearBufferfv(GL_DEPTH, 0, float_ones);

        g_geometry_shader.use();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        const glm::mat4 model;

        // CAMERA
        update_camera(model);

        // Render the scene. (Also binds relevant textures)
        render_scene();

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

    }


    void
    Engine::deferred_lighting_pass(const Shader &lighting_shader, const GBuffer &gbuffer, const FBO &render_fbo) const {


        render_fbo.bind();

        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat float_ones = 1.0f;
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_COLOR, 1, black);
        glClearBufferfv(GL_DEPTH, 0, &float_ones);

        lighting_shader.use();
        gbuffer.bind_for_lighting_pass();

        glUniform1ui(glGetUniformLocation(lighting_shader.program_, "showNormals"), show_normals_);
        glUniform3fv(glGetUniformLocation(lighting_shader.program_, "viewPos"), 1, glm::value_ptr(camera_.Position));

        quad_.draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    void Engine::bloom_pass(const FBO &render_fbo, const std::array<FBO, 2> &filter_fbos, const Shader &shader_filter,
                            const Shader &shader_combine) const {


        //-----------------------------------------------
        // Filter
        shader_filter.use();

        glBindVertexArray(quad_.vao_); // Bind the quad's VAO

        filter_fbos[0].bind(); // Use the first filter FBO

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo.textures_[1]); // Bind the brightpass texture

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        filter_fbos[1].bind(); // Use the second filter FBO

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

        // Render to default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo.textures_[0]); // And the original HDR image

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, filter_fbos[1].textures_[0]); // Use the texture in the second filter FBO


        // Finally draw the result
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    void Engine::render_scene() const {
        for (const auto &model : models_) {
            model.draw();
        }
    }


    void Engine::update_camera(const glm::mat4 &model) const {

        // Set up the projection matrix and feed the data to the uniform block object
        const glm::mat4 projection = glm::perspective(camera_.Zoom,
                                                      (GLfloat) window_width_ / (GLfloat) window_height_,
                                                      0.1f,
                                                      1000.0f);
        // To the same for the view matrix
        const glm::mat4 view = camera_.GetViewMatrix();

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_transforms_);
        glm::mat4 *const matrices = reinterpret_cast<glm::mat4 *>(glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                                   0,
                                                                                   2 * sizeof(glm::mat4),
                                                                                   GL_MAP_WRITE_BIT |
                                                                                   GL_MAP_INVALIDATE_BUFFER_BIT));
        matrices[0] = projection * view;
        matrices[1] = model;

        glUnmapBuffer(GL_UNIFORM_BUFFER);

    }


    void Engine::update_lights(const Shader &shader) const {

        shader.use();

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, lights_ubo_);

        Light *const lights = reinterpret_cast<Light *>(glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                         0,
                                                                         nrOfLights_ * sizeof(Light),
                                                                         GL_MAP_WRITE_BIT |
                                                                         GL_MAP_INVALIDATE_BUFFER_BIT));

        const GLfloat time = static_cast<GLfloat>(glfwGetTime()) * 0.05f;
        const GLfloat threshold = std::pow(2.0f, 16.0f) / 30.0f;

        for (unsigned int i = 0; i < nrOfLights_; i++) {
            const float i_f = ((float) i - 7.5f) * 0.1f + 0.3f;

            lights[i].position = glm::vec3(
                    100.0f * std::sin(time * 1.1f + (5.0f * i_f)) * std::cos(time * 2.3f + (9.0f * i_f)),
                    15.0f,
                    100.0f * std::sin(time * 1.5f + (6.0f * i_f)) * std::cos(time * 1.9f + (11.0f *
                                                                                            i_f)));
            glm::vec3 color = glm::vec3(std::cos(i_f * 14.0f) * 0.5f + 0.8f,
                                        std::sin(i_f * 17.0f) * 0.5f + 0.8f,
                                        std::sin(i_f * 13.0f) * std::cos(i_f * 19.0f) * 0.5f + 0.8f);

            lights[i].color = color;
            lights[i].linear = light_linear_factor_;
            lights[i].quadratic = light_quadratic_factor_;

            const GLfloat max_brightness = std::fmax(std::fmax(color.r, color.g), color.b);
            lights[i].radius = (-light_linear_factor_ +
                                std::sqrt(light_linear_factor_ * light_linear_factor_ -
                                          4 * light_quadratic_factor_ *
                                          (1.0f - threshold * max_brightness))) /
                               (2 * light_quadratic_factor_);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

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

        if (engine->camera_enabled_) {
            engine->camera_.ProcessMouseMovement(xoffset, yoffset);
        }

    }


    void Engine::handle_input(float delta_time) {

        ImGuiIO &io = ImGui::GetIO();

        if (io.KeysDown[GLFW_KEY_ESCAPE])
            glfwSetWindowShouldClose(window_, GL_TRUE);

        if (io.KeysDown[GLFW_KEY_C]) {
            camera_enabled_ = true;
        }
        if (io.KeysDown[GLFW_KEY_V]) {
            camera_enabled_ = false;
        }

        // Camera controls
        if (io.KeysDown[GLFW_KEY_W])
            camera_.ProcessKeyboard(FORWARD, delta_time);
        if (io.KeysDown[GLFW_KEY_S])
            camera_.ProcessKeyboard(BACKWARD, delta_time);
        if (io.KeysDown[GLFW_KEY_A])
            camera_.ProcessKeyboard(LEFT, delta_time);
        if (io.KeysDown[GLFW_KEY_D])
            camera_.ProcessKeyboard(RIGHT, delta_time);

        if (io.KeysDown[GLFW_KEY_N]) {
            show_normals_ = 1;
        }
        if (io.KeysDown[GLFW_KEY_M]) {
            show_normals_ = 0;
        }
        if (io.KeysDown[GLFW_KEY_F]) {
            should_render_deferred_ = false;
        }
        if (io.KeysDown[GLFW_KEY_G]) {
            should_render_deferred_ = true;
        }

        if (io.KeysDown[GLFW_KEY_1]) {
            light_linear_factor_ = 0.7f;
            light_quadratic_factor_ = 1.8f;
        }
        if (io.KeysDown[GLFW_KEY_2]) {
            light_linear_factor_ = 0.35f;
            light_quadratic_factor_ = 0.44f;
        }
        if (io.KeysDown[GLFW_KEY_3]) {
            light_linear_factor_ = 0.14f;
            light_quadratic_factor_ = 0.07f;
        }
        if (io.KeysDown[GLFW_KEY_4]) {
            light_linear_factor_ = 0.045f;
            light_quadratic_factor_ = 0.0075f;
        }
        if (io.KeysDown[GLFW_KEY_5]) {
            light_linear_factor_ = 0.022f;
            light_quadratic_factor_ = 0.0019f;
        }
        if (io.KeysDown[GLFW_KEY_6]) {
            light_linear_factor_ = 0.012f;
            light_quadratic_factor_ = 0.0007f;
        }


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
