//
// Created by Niclas Olmenius on 2016-11-02.
//


#pragma once

#include <GL/glew.h>
#define nrOfTextureIds = 3;

namespace rengine {

class GBuffer {

public:
    explicit GBuffer(int width, int height);
    ~GBuffer();

    void use();

    GLuint tex_[3];
    GLuint position_, normal_, albedo_spec_, depth_;

private:
    GLuint g_buffer_;
    int width_, height_;

};

}
