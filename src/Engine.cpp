//
// Created by niclas on 2016-10-21.
//

#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <memory>
#include "Engine.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GBuffer.h>
#include <Light.h>
#include <array>

namespace rengine {

    bool Engine::instantiated_ = false;

    double Engine::last_x;
    double Engine::last_y;
    bool Engine::keys_[1024];
    bool Engine::first_mouse_movement_;

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

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cout << "Failed to initialize GLEW" << std::endl;
            return -1;
        }

        printf("GL version:      %s\n", glGetString(GL_VERSION));

        glfwGetFramebufferSize(window_, &window_width_, &window_height_);
        glViewport(0, 0, window_width_, window_height_);

        return true;

    }

    bool Engine::enable_gl_features() {
//    glEnable(GL_DEPTH_TEST);

        return true;
    }

    bool Engine::setup_camera() {
        camera_ = Camera{glm::vec3(0.0f, 0.0f, 3.0f)};
        first_mouse_movement_ = true;

        return true;
    }

    bool Engine::compile_shaders() {

        return true;
    }

    void Engine::run() {

        // Create and compile shaders
        auto geometry_shader = Shader{"../assets/shaders/g_geometry.vert", "../assets/shaders/g_geometry.frag"};
        auto lighting_shader = Shader{"../assets/shaders/hdr_lighting.vert", "../assets/shaders/hdr_lighting.frag"};
        auto filter_shader = Shader{"../assets/shaders/gaussian_blur.vert", "../assets/shaders/gaussian_blur.frag"};
        auto combine_bloom_shader = Shader{"../assets/shaders/bloom_combine.vert",
                                           "../assets/shaders/bloom_combine.frag"};

        lighting_shader.use();
        glUniform1i(glGetUniformLocation(lighting_shader.program_, "gPosition"), 0);
        glUniform1i(glGetUniformLocation(lighting_shader.program_, "gNormal"), 1);
        glUniform1i(glGetUniformLocation(lighting_shader.program_, "gAlbedoSpec"), 2);


        // Set up lights
        // - Colors
        std::vector<glm::vec3> light_positions;
        std::vector<glm::vec3> light_colors;

        setup_lights(light_positions, light_colors);

        // Use the GBuffer geometry shader
        geometry_shader.use();

        // ------------------------------------------------------------------------------------
        // Set up the uniform transform block for each shader
        // should probably do this in the shader manager somehow
        GLuint transform_ubo = glGetUniformBlockIndex(geometry_shader.program_, "TransformBlock");
        glUniformBlockBinding(geometry_shader.program_, transform_ubo, 0);

        GLuint ubo_transforms;
        glGenBuffers(1, &ubo_transforms);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_transforms, 0, 2 * sizeof(glm::mat4));

        // ------------------------------------------------------------------------------------
        // Set up framebuffers...
        // FBO objects?

        const auto gbuffer = GBuffer{window_width_, window_height_};
        const auto render_fbo = FBO{window_width_, window_height_, 2, true};
        const auto filter_fbos = std::array<FBO, 2>{{FBO{window_width_, window_height_, 1, false},
                                                            FBO{window_width_, window_height_, 1, false}}};

        // Create quad for rendering the final image
        auto quad = Quad{};

//        quad_ = Quad{};


        // ------------------------------------------------------------------------------------

        GLfloat current_frame_time = 0.0f;
        GLfloat delta_time = 0.0f;
        GLfloat last_frame_time = 0.0f;

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

            // CAMERA
            update_camera(ubo_transforms);

            // LIGHTS
            update_lights(light_positions, current_frame_time);

            // ------------------------------------------------------
            // RENDERING

            // Geometry pass
            geometry_pass(geometry_shader, gbuffer);

            // Lighting pass TODO Refactor to not use as many parameters
            lighting_pass(camera_, lighting_shader, light_positions, light_colors, gbuffer, render_fbo,
                          quad);

            bloom_pass(render_fbo, filter_fbos, filter_shader, combine_bloom_shader, quad);


            // Swap the screen buffers
            glfwSwapBuffers(window_);
        }

    }


    void Engine::geometry_pass(Shader &g_geometry_shader, const GBuffer &gbuffer) const {

        gbuffer.bind_for_geometry_pass();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_geometry_shader.use();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);

        // Draw the loaded model
        glm::mat4 model;
        model = glm::translate(model,
                               glm::vec3(0.0f, -3.0f,
                                         0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.25f));    // It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(g_geometry_shader.program_, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));

        // Render the scene. (Also binds relevant textures)
        render_scene(g_geometry_shader.program_);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Engine::lighting_pass(const Camera &camera, const Shader &lighting_shader,
                               const std::vector<glm::vec3> &lightPositions,
                               const std::vector<glm::vec3> &lightColors, const GBuffer &gbuffer, const FBO &render_fbo,
                               const Quad &quad) const {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gbuffer.bind_for_lighting_pass();
        render_fbo.bind_draw();


        lighting_shader.use();

        // Lights
        // -------------------------------------------------------------------------------------------

        const float temp_time = static_cast<float>(glfwGetTime());


        // TODO do it more like in the OpenGL superbible, seems way more efficient
        for (int i = 0; i < lightPositions.size(); i++) {
            const glm::vec3 pos(lightPositions[i].x, lightPositions[i].y, lightPositions[i].z * cosf(temp_time));
            glUniform3fv(glGetUniformLocation(lighting_shader.program_,
                                              ("lights[" + std::to_string(i) + "].Position").c_str()),
                         1,
                         glm::value_ptr(pos));
            glUniform3fv(
                    glGetUniformLocation(lighting_shader.program_, ("lights[" + std::to_string(i) + "].Color").c_str()),
                    1,
                    glm::value_ptr(lightColors[i]));
            // Update attenuation parameters and calculate radius
            const GLfloat
                    constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const GLfloat linear = 0.22;
            const GLfloat quadratic = 0.20;
            glUniform1f(glGetUniformLocation(lighting_shader.program_,
                                             ("lights[" + std::to_string(i) + "].Linear").c_str()), linear);
            glUniform1f(glGetUniformLocation(lighting_shader.program_,
                                             ("lights[" + std::to_string(i) + "].Quadratic").c_str()), quadratic);
        }

        glUniform3fv(glGetUniformLocation(lighting_shader.program_, "viewPos"), 1, &camera.Position[0]);

        quad.draw();

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


    }


    bool Engine::load_scene() {

        models_.push_back(Model{"../assets/models/nano/nanosuit.obj"});
//        models_.push_back(Model{"../assets/bible/objects/dragon.sbm"});
//        models_.push_back(Model{"../assets/models/blender/Blenderman.dae"});
//        models_.push_back(Model{"../assets/models/e.dae"});
//        models_.push_back(Model{"../assets/models/bmw/bmw.obj"});

        return true;

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

    }

    void Engine::update_camera(GLuint ubo_transforms) {

        // Set up the projection matrix and feed the data to the uniform block object
        glm::mat4 projection = glm::perspective(camera_.Zoom, (GLfloat) window_width_ / (GLfloat) window_height_, 0.1f,
                                                1000.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // To the same for the view matrix
        // TODO refactor this to a update_camera function
        glm::mat4 view = camera_.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferSubData(
                GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }

    void Engine::setup_lights(std::vector<glm::vec3> &light_positions, std::vector<glm::vec3> &light_colors) const {
        const GLuint NR_LIGHTS = 32;
        srand(13);

        for (GLuint i = 0; i < NR_LIGHTS; i++) {
            // Calculate slightly random offsets
            GLfloat xPos = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
            GLfloat yPos = ((rand() % 100) / 100.0f) * 6.0f - 4.0f;
            GLfloat zPos = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
            light_positions.push_back(glm::vec3(xPos, yPos, zPos));

            // Also calculate random color
            GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5f; // Between 0.5 and 1.0
            GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5f; // Between 0.5 and 1.0
            GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5f; // Between 0.5 and 1.0
            light_colors.push_back(glm::vec3(rColor, gColor, bColor));
        }
    }

    void Engine::update_lights(std::vector<glm::vec3> &light_positions, GLfloat time) {

//        light_positions[0] = camera_.Position;

//        for(int i = 0; i < light_positions.size(); i++) {
//            // 1 0 0
//            light_positions[i].z *= 5.0f*cosf(time);
//        }

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

//        const float fps = 1000.0f / frametime;
//        char titlestring[200];
//        sprintf(titlestring, "Deferred renderer, %.2f ms/frame (%.1f FPS)", frametime, fps);
//        glfwSetWindowTitle(window_, titlestring);

        static double t0 = 0.0;
        static int frames = 0;
        static double fps = 0.0;
        static double frametime = 0.0;
        static char titlestring[200];

        // Get current time
//        t = glfwGetTime();  // Gets number of seconds since glfwInit()
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