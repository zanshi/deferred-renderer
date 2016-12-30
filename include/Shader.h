#pragma once

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

#include <GL/glew.h>

namespace rengine {

    class Shader {

    public:
        Shader(std::string vert_path, std::string frag_path);
        GLuint program_;

        void use() const;

    };

}
