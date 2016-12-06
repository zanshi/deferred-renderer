//
// Created by niclas on 2016-12-06.
//

#include <glm/detail/type_mat.hpp>
#include <Quad.h>
#include <GBuffer.h>
#include <glm/gtc/type_ptr.hpp>
#include "DeferredRenderer.h"


namespace rengine {

    DeferredRenderer::DeferredRenderer(int width, int height, const Shader &geometry_shader,
                                       const Shader &lighting_shader) : window_width_{width}, window_height_{height},
                                                                        geometry_shader_{geometry_shader},
                                                                        lighting_shader_{lighting_shader},
                                                                        g_buffer_{width, height} {
    }

    DeferredRenderer::~DeferredRenderer() {

    }


    void DeferredRenderer::setup() {

        // Set up gBuffer stuff
        lighting_shader_.use();
        glUniform1i(glGetUniformLocation(lighting_shader_.program_, "gPosition"), 0);
        glUniform1i(glGetUniformLocation(lighting_shader_.program_, "gNormal"), 1);
        glUniform1i(glGetUniformLocation(lighting_shader_.program_, "gAlbedoSpec"), 2);


        // Set up the transform ubo
        geometry_shader_.use();
        // ------------------------------------------------------------------------------------
        // Set up the uniform transform block for each shader
        // should probably do this in the shader manager somehow
        GLuint transform_ubo = glGetUniformBlockIndex(geometry_shader_.program_, "TransformBlock");
        glUniformBlockBinding(geometry_shader_.program_, transform_ubo, 0);


        glGenBuffers(1, &ubo_transforms_);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms_);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_transforms_, 0, 2 * sizeof(glm::mat4));


        // Setup the gBuffer
        g_buffer_.setup();

        // Set up the quad so that we can render off screen
        quad_ = Quad{};


    }

    void DeferredRenderer::render(const std::vector<Model> models, const FBO &render_fbo,
                                  const std::vector<glm::vec3> &light_positions,
                                  const std::vector<glm::vec3> &light_colors, const Camera &camera) const {

        glViewport(0, 0, window_width_, window_height_);

        // Geometry pass
        geometry_pass(models, camera);

        // Lighting pass TODO Refactor to not use as many parameters
        lighting_pass(camera, light_positions, light_colors, render_fbo);

    }


    void DeferredRenderer::update_proj_view(const Camera &camera) const {

        // Set up the projection matrix and feed the data to the uniform block object
        const glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat) window_width_ / (GLfloat) window_height_, 0.1f,
                                                1000.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
//        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // To the same for the view matrix
        const glm::mat4 view = camera.GetViewMatrix();
//        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transforms);
        glBufferSubData(
                GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }

    
    
    void DeferredRenderer::geometry_pass(const std::vector<Model> models, const Camera& camera) const {
        g_buffer_.bind_for_geometry_pass();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        geometry_shader_->use();

        glEnable(GL_DEPTH_TEST);
//        glDepthFunc(GL_LEQUAL);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);

        // CAMERA
        update_proj_view(camera);

        // Draw the loaded model
        glm::mat4 model_matrix;
//        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
//        model = glm::scale(model, glm::vec3(0.25f));    // It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(geometry_shader_->program_, "model"), 1, GL_FALSE,
                           glm::value_ptr(model_matrix));

        // Render the scene. (Also binds relevant textures)
        for (auto && model: models) {
            model.draw(geometry_shader_->program_);
        }


//        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    void DeferredRenderer::lighting_pass(const Camera &camera, const std::vector<glm::vec3>(light_positions),
                                         const std::vector<glm::vec3>(light_colors), const FBO &render_fbo) const {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        g_buffer_.bind_for_lighting_pass();
        render_fbo.bind_draw();


        lighting_shader_->use();

        glUniform1ui(glGetUniformLocation(lighting_shader_->program_, "showNormals"), show_normals_);


        // Lights
        // -------------------------------------------------------------------------------------------

        const float time = static_cast<float>(glfwGetTime() * 0.1);

        const GLuint phase = 40;

        // TODO do it more like in the OpenGL superbible, seems way more efficient
        for (int i = 0; i < lightPositions.size(); i++) {

            float i_f = ((float) i - 7.5f) * 0.1f + 0.3f;
            // t = 0.0f;
            const glm::vec3 pos = glm::vec3(
                    100.0f * std::sin(time * 1.1f + (5.0f * i_f)) * std::cos(time * 2.3f + (9.0f * i_f)),
                    15.0f,
                    100.0f * std::sin(time * 1.5f + (6.0f * i_f)) * std::cos(time * 1.9f + (11.0f *
                                                                                            i_f))); // 300.0f * std::sin(time * i_f * 0.7f) * std::cos(time * i_f * 0.9f) - 600.0f);
            const glm::vec3 color = glm::vec3(std::cos(i_f * 14.0f) * 0.5f + 0.8f,
                                              std::sin(i_f * 17.0f) * 0.5f + 0.8f,
                                              std::sin(i_f * 13.0f) * std::cos(i_f * 19.0f) * 0.5f + 0.8f);

//            const glm::vec3 pos = glm::rotateY(lightPositions[i], temp_time + phase * i);
//            const glm::vec3 pos = lightPositions[i];


//            const glm::vec3 pos(lightPositions[i].x, lightPositions[i].y, lightPositions[i].z * rot * (i+1));
            glUniform3fv(glGetUniformLocation(lighting_shader_.program_,
                                              ("lights[" + std::to_string(i) + "].Position").c_str()),
                         1,
                         glm::value_ptr(pos));
            glUniform3fv(
                    glGetUniformLocation(lighting_shader_.program_, ("lights[" + std::to_string(i) + "].Color").c_str()),
                    1,
                    glm::value_ptr(color));

            glUniform1f(glGetUniformLocation(lighting_shader_.program_,
                                             ("lights[" + std::to_string(i) + "].Linear").c_str()),
                        light_linear_factor_);
            glUniform1f(glGetUniformLocation(lighting_shader_.program_,
                                             ("lights[" + std::to_string(i) + "].Quadratic").c_str()),
                        light_quadratic_factor_);
        }

        glUniform3fv(glGetUniformLocation(lighting_shader_.program_, "viewPos"), 1, glm::value_ptr(camera.Position));

        quad.draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        
    };
    


}
