//
// Created by niclas on 2016-12-06.
//


#pragma once

#include "IRenderer.h"
#include "Shader.h"
#include <memory>
#include "camera.h"


namespace rengine {

    class DeferredRenderer : public IRenderer {

    public:
        DeferredRenderer(int width, int height, const Shader &geometry_shader,
                         const Shader &lighting_shader);

        ~DeferredRenderer();

        void setup() override;

        void render(const std::vector<Model> models, const FBO &render_fbo,
                    const std::vector<glm::vec3> &light_positions, const std::vector<glm::vec3> &light_colors,
                    const Camera &camera) const override;

    private:

        std::unique_ptr<Shader> geometry_shader_, lighting_shader_;
        GBuffer g_buffer_;
        GLuint ubo_transforms_;

        void lighting_pass(const Camera &camera, const std::vector<glm::vec3>(light_positions),
                           const std::vector<glm::vec3>(light_colors), const FBO &render_fbo) const;

        void geometry_pass(const std::vector<Model> models, const Camera& camera) const;

        void update_proj_view(const Camera &camera) const;


    };
}
