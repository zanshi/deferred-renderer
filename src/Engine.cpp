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
#include <Quad.h>

namespace rengine {

bool Engine::instantiated_ = false;

Engine::Engine(int width, int height)
    : screen_width_{width}, screen_height_{height} {
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
  glfwDestroyWindow(window);
  glfwTerminate();

}

void Engine::run() {

  // Set up camera
  Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));



//        Shader shader("../assets/shaders/plain.vert", "../assets/shaders/plain.frag");

  // Create and compile shaders
  auto shader = Shader{"../assets/shaders/shader.vert", "../assets/shaders/shader.frag"};
  auto g_geometry_shader = Shader{"../assets/shaders/g_geometry.vert", "../assets/shaders/g_geometry.frag"};
  auto g_lighting = Shader{"../assets/shaders/g_lighting.vert", "../assets/shaders/g_lighting.frag"};

  g_lighting.use();
  glUniform1i(glGetUniformLocation(g_lighting.program_, "gPosition"), 0);
  glUniform1i(glGetUniformLocation(g_lighting.program_, "gNormal"), 1);
  glUniform1i(glGetUniformLocation(g_lighting.program_, "gAlbedoSpec"), 2);

  // Use the GBuffer geometry shader
  g_geometry_shader.use();
  current_program_ = g_geometry_shader.program_;



  // Set up lights

  // - Colors
  const GLuint NR_LIGHTS = 32;
  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightColors;
  srand(13);
  for (GLuint i = 0; i < NR_LIGHTS; i++) {
    // Calculate slightly random offsets
    GLfloat xPos = ((rand()%100)/100.0f)*6.0f - 3.0f;
    GLfloat yPos = ((rand()%100)/100.0f)*6.0f - 4.0f;
    GLfloat zPos = ((rand()%100)/100.0f)*6.0f - 3.0f;
    lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

    // Also calculate random color
    GLfloat rColor = ((rand()%100)/200.0f) + 0.5f; // Between 0.5 and 1.0
    GLfloat gColor = ((rand()%100)/200.0f) + 0.5f; // Between 0.5 and 1.0
    GLfloat bColor = ((rand()%100)/200.0f) + 0.5f; // Between 0.5 and 1.0
    lightColors.push_back(glm::vec3(rColor, gColor, bColor));
  }

  // ------------------------------------------------------------------------------------
  // Set up the uniform transform block for each shader
  // should probably do this in the shader manager somehow
  GLuint transform_ubo = glGetUniformBlockIndex(g_geometry_shader.program_, "TransformBlock");
  glUniformBlockBinding(g_geometry_shader.program_, transform_ubo, 0);

  GLuint ubo_transforms;
  glGenBuffers(1, &ubo_transforms);

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
  glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_transforms, 0, 2*sizeof(glm::mat4));

  // ------------------------------------------------------------------------------------


  // Set up the projection matrix and feed the data to the uniform block object
  glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat) screen_width_/(GLfloat) screen_height_, 0.1f,
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
  // FBO objects?

  auto gbuffer = GBuffer{screen_width_, screen_height_};

  // Create quad
  auto quad = Quad{};

  // ------------------------------------------------------------------------------------

  float time;

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();


    // INPUT

    // Handle input


//    time = static_cast<float>(glfwGetTime());


    // RENDERING

    // Geometry pass
    geometry_pass(g_geometry_shader, gbuffer);

    // Lighting pass
    lighting_pass(camera, g_lighting, lightPositions, lightColors, gbuffer);

    // Draw final result to screen
    quad.draw();

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

}
void Engine::geometry_pass(Shader &g_geometry_shader, GBuffer &gbuffer) const {
  gbuffer.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  g_geometry_shader.use();

  // Draw the loaded model
  glm::mat4 model;
  model = translate(model,
                    glm::vec3(0.0f, -3.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
  model = scale(model, glm::vec3(0.25f));    // It's a bit too big for our scene, so scale it down
  glUniformMatrix4fv(glGetUniformLocation(g_geometry_shader.program_, "model"), 1, GL_FALSE,
                     value_ptr(model));

  // Render the scene. (Also binds relevant textures)
  render_scene(g_geometry_shader.program_);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Engine::lighting_pass(const Camera &camera,
                           Shader &g_lighting,
                           const std::vector<glm::vec3> &lightPositions,
                           const std::vector<glm::vec3> &lightColors,
                           const GBuffer &gbuffer) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  g_lighting.use();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gbuffer.position_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gbuffer.normal_);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gbuffer.albedo_spec_);


  // Lights
  // -------------------------------------------------------------------------------------------

  // TODO do it more like in the OpenGL superbible, seems way more efficient
  for (int i = 0; i < lightPositions.size(); i++) {

    glUniform3fv(glGetUniformLocation(g_lighting.program_, ("lights[" + std::__1::to_string(i) + "].Position").c_str()),
                 1,
                 &lightPositions[i][0]);
    glUniform3fv(glGetUniformLocation(g_lighting.program_, ("lights[" + std::__1::to_string(i) + "].Color").c_str()),
                 1,
                 &lightColors[i][0]);
    // Update attenuation parameters and calculate radius
    const GLfloat
        constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
    const GLfloat linear = 0.7;
    const GLfloat quadratic = 1.8;
    glUniform1f(glGetUniformLocation(g_lighting.program_,
                                     ("lights[" + std::__1::to_string(i) + "].Linear").c_str()), linear);
    glUniform1f(glGetUniformLocation(g_lighting.program_,
                                     ("lights[" + std::__1::to_string(i) + "].Quadratic").c_str()), quadratic);
  }

  glUniform3fv(glGetUniformLocation(g_lighting.program_, "viewPos"), 1, &camera.Position[0]);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

bool Engine::load_scene() {

  models_.push_back(Model{"../assets/models/nano/nanosuit.obj"});

  return true;

}

void Engine::render_scene(GLuint shader_program) const {
  for (auto &&model : models_) {
    model.draw(shader_program);
  }

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

  printf("GL version:      %s\n", glGetString(GL_VERSION));

  glfwGetFramebufferSize(window, &screen_width_, &screen_height_);
  glViewport(0, 0, screen_width_, screen_height_);

  return true;

}

bool Engine::enable_gl_features() {
//    glEnable(GL_DEPTH_TEST);

  return true;
}

bool Engine::setup_camera() {
  return true;
}

bool Engine::compile_shaders() {

  return true;
}

void Engine::error_callback(int error, const char *description) {
  fputs(description, stderr);
}

void Engine::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

}