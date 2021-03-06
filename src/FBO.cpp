//
// Created by Niclas Olmenius
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

        // Create frame buffer textures
        for (GLuint i = 0; i < nr_textures; i++) {
            glBindTexture(GL_TEXTURE_2D, textures_[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, width_, height_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures_[i], 0);

            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        if (gen_depth_buffer) {
            glGenTextures(1, &tex_depth_);
            glBindTexture(GL_TEXTURE_2D, tex_depth_);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width_, height_);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_depth_, 0);

        }

        glDrawBuffers(nr_textures, attachments.data());

        // Check for completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    FBO::~FBO() {

    }

    void FBO::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, width_, height_);
    }
}
