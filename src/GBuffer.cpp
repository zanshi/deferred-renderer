//
// Created by Niclas Olmenius on 2016-11-02.
//

#include <iostream>
#include "GBuffer.h"

namespace rengine {

    GBuffer::GBuffer(int width, int height) : width_{width}, height_{height}{

    }

    GBuffer::~GBuffer() {}


    void GBuffer::setup() {

        glGenFramebuffers(1, &g_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);

        // - Position color buffer
        glGenTextures(1, &position_);
        glBindTexture(GL_TEXTURE_2D, position_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, position_, 0);

        // - Normal color buffer
        glGenTextures(1, &normal_);
        glBindTexture(GL_TEXTURE_2D, normal_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_, 0);

        // - Color + Specular color buffer
        glGenTextures(1, &albedo_spec_);
        glBindTexture(GL_TEXTURE_2D, albedo_spec_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, albedo_spec_, 0);

        static const GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glGenRenderbuffers(1, &depth_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GBuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);
        glViewport(0, 0, width_, height_);
    }

    void GBuffer::bind_for_lighting_pass() const {
//        bind_read();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, position_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, albedo_spec_);

    }

    void GBuffer::bind_for_geometry_pass() const {
        bind();
    }

}