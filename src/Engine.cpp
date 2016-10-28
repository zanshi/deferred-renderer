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

Engine::Engine(int width, int height)
        :screen_width_{width}, screen_height_{height} { }

bool Engine::init()
{

    if (!init_gl_context())
        return false;

    if (!load_scene())
        return false;

//        Assimp::Importer importer;
//
//        glm::vec4 vec4;

    return true;

}

void Engine::run()
{

    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

//        Shader shader("../assets/shaders/plain.vert", "../assets/shaders/plain.frag");
    Shader shader("../assets/shaders/shader.vert", "../assets/shaders/shader.frag");
    current_program_ = shader.program_;

    shader.use();

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

//            glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(camera.Zoom, (float) screen_width_/(float) screen_height_, 0.1f,
                100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.program_, "projection"), 1, GL_FALSE,
                glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.program_, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Draw the loaded model
        glm::mat4 model;
        model = glm::translate(model,
                glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model,
                glm::vec3(0.2f, 0.2f, 0.2f));    // It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.program_, "model"), 1, GL_FALSE, glm::value_ptr(model));

        render_scene();

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

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

    // enable opengl features
    glEnable(GL_DEPTH_TEST);

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