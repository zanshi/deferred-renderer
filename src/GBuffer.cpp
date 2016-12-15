//
// Created by Niclas Olmenius on 2016-11-02.
//

#include <iostream>
#include "GBuffer.h"

namespace rengine {


    GBuffer::GBuffer(int width, int height)
            : width_{width}, height_{height} {

        glGenFramebuffers(1, &g_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);


        glGenTextures(3, tex_);

        // Color and normals
        glBindTexture(GL_TEXTURE_2D, tex_[0]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, width_, height_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Positions and specular
        glBindTexture(GL_TEXTURE_2D, tex_[1]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width_, height_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Depth
        glBindTexture(GL_TEXTURE_2D, tex_[2]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);


        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_[0], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_[1], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_[2], 0);

        static const GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Gbuffer not complete!" << std::endl;
            std::cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    GBuffer::~GBuffer() {}

    void GBuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);
        glViewport(0, 0, width_, height_);
    }

    void GBuffer::bind_for_lighting_pass() const {

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_[1]);

    }

    void GBuffer::bind_for_geometry_pass() const {
        bind();
    }


}