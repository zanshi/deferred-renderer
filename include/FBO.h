//
// Created by Niclas Olmenius on 2016-11-02.
//


#pragma once

#include <GL/glew.h>
#include <vector>

namespace rengine {


    class FBO {

    public :

        FBO(int width, int height, GLuint nr_textures = 1, bool gen_depth_buffer = false);

        ~FBO();

        void bind() const;

        std::vector<GLuint> textures_;
        GLuint tex_depth_;

    private:

        GLuint fbo_;
        int width_, height_;

        GLuint tex_scene_;
        GLuint tex_brightpass_;
    };

}

