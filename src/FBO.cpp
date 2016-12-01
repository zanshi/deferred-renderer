//
// Created by Niclas Olmenius on 2016-11-02.
//

#include <iostream>
#include "FBO.h"

namespace rengine {


    FBO::FBO(int width, int height, GLuint nr_textures, bool gen_depth_buffer) : width_{width}, height_{height} {

        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        textures_.reserve(nr_textures);

        glGenTextures(nr_textures, textures_.data());
        std::vector<GLenum> attachments;

        for (GLuint i = 0; i < nr_textures; i++) {
            glBindTexture(GL_TEXTURE_2D, textures_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures_[i], 0);

            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        glDrawBuffers(nr_textures, attachments.data());


        if (gen_depth_buffer) {
            glGenRenderbuffers(1, &tex_depth_);
            glBindRenderbuffer(GL_RENDERBUFFER, tex_depth_);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tex_depth_);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    FBO::~FBO() {

    }

    void FBO::bind_read() const {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);

    }

    void FBO::bind_draw() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, width_, height_);
    }
}
