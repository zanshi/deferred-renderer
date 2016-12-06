//
// Created by Niclas Olmenius on 2016-11-02.
//


#pragma once

#include <GL/glew.h>
#define nrOfTextureIds = 3;

namespace rengine {

class GBuffer {

public:

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_ALBEDO,
        GBUFFER_NUM_TEXTURES
    };

    GBuffer(int width, int height);
    ~GBuffer();

    void setup();

    void bind_for_geometry_pass() const;
    void bind_for_lighting_pass() const;
    void bind_for_final_pass() const;
    void bind() const;

    GLuint tex_[3];
    GLuint position_, normal_, albedo_spec_, depth_;

private:
    GLuint g_buffer_;
    int width_, height_;

};

}
