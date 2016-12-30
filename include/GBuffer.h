//
// Created by Niclas Olmenius
//

#pragma once

#include <GL/glew.h>

namespace rengine {

class GBuffer {

public:

    GBuffer(int width, int height);
    ~GBuffer();

    void bind_for_geometry_pass() const;
    void bind_for_lighting_pass() const;
    void bind_for_final_pass() const;
    void bind() const;

    GLuint tex_[3];

private:
    GLuint g_buffer_;
    int width_, height_;

};

}
