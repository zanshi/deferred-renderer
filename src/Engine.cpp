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

namespace rengine {

bool Engine::instantiated_ = false;

Engine::Engine(int width, int height)
        :screen_width_{width}, screen_height_{height}
{
    assert(!instantiated_); // prevent more than one instance from being started
    instantiated_ = true;
}

Engine::~Engine()
{
    instantiated_ = false;
}

void Engine::start_up()
{

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

void Engine::shut_down()
{
    glfwDestroyWindow(window);
    glfwTerminate();

}

void Engine::run()
{

    // Set up camera
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

//        Shader shader("../assets/shaders/plain.vert", "../assets/shaders/plain.frag");

    // Create and compile shaders
    Shader shader("../assets/shaders/shader.vert", "../assets/shaders/shader.frag");
    current_program_ = shader.program_;

    shader.use();


    // ------------------------------------------------------------------------------------
    // Set up the uniform transform block for each shader
    // should probably do this in the shader manager somehow
    GLuint transform_ubo = glGetUniformBlockIndex(shader.program_, "TransformBlock");
    glUniformBlockBinding(shader.program_, transform_ubo, 0);

    GLuint ubo_transforms;
    glGenBuffers(1, &ubo_transforms);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_transforms, 0, 2 * sizeof(glm::mat4));

    // ------------------------------------------------------------------------------------

    // Set up the projection matrix and feed the data to the uniform block object
    glm::mat4 projection = glm::perspective(camera.Zoom, (float) screen_width_/(float) screen_height_, 0.1f,
            100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // To the same for the view matrix
    // TODO refactor this to a update_camera function
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
    glBufferSubData(
            GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // ------------------------------------------------------------------------------------
    // Set up framebuffers...




    // ------------------------------------------------------------------------------------



    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // TODO 1. Geometry pass...


        // Draw the loaded model
        glm::mat4 model;
        model = glm::translate(model,
                glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model,
                glm::vec3(0.2f, 0.2f, 0.2f));    // It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.program_, "model"), 1, GL_FALSE,
                glm::value_ptr(model));

        render_scene();

        // TODO 2. Lighting pass...


        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

}

bool Engine::load_scene()
{

//        Assimp::Importer importer;
//
//        scene_ = importer.ReadFile("../assets/models/solid.nff",
//                aiProcessPreset_TargetRealtime_MaxQuality);
//
//        // check if load failed
//        if (!scene_) {
//            return false;
//        }

//        Model model("../assets/models/solid.nff");

    models_.push_back(Model{"../assets/models/nano/nanosuit.obj"});

    return true;

}

void Engine::render_scene() const
{
    for (auto&& model : models_) {
        model.draw(current_program_);
    }

}

bool Engine::init_gl_context()
{

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return false;
//        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(screen_width_, screen_height_, "Deferred renderer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit()!=GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &screen_width_, &screen_height_);
    glViewport(0, 0, screen_width_, screen_height_);

    return true;

}

bool Engine::enable_gl_features()
{
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool Engine::setup_camera()
{
    return true;
}

bool Engine::compile_shaders()
{

    return true;
}

void Engine::error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

void Engine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

}