//
// Created by niclas on 2016-12-06.
//

#pragma once

#include <vector>
#include "Model.h"
#include "FBO.h"
#include "Quad.h"


namespace rengine {

    class IRenderer {

    public:

        virtual ~IRenderer() {};

        virtual void setup() = 0;

        virtual void render(const std::vector<Model> models, const FBO &render_fbo,
                            const std::vector<glm::vec3> &light_positions, const std::vector<glm::vec3> &light_colors,
                            const Camera &camera) const = 0;


    protected:
        int window_width_;
        int window_height_;
        Quad quad_;


    };


}