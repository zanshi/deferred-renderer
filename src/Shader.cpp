//
// Created by Niclas Olmenius on 2016-10-27.
//


#include "Shader.h"

namespace rengine {


    Shader::Shader(std::string vert_path, std::string frag_path)
    {
        // 1. Retrieve the vertex/fragment source code from filePath
        std::string vertex_code;
        std::string fragment_code;
        std::ifstream v_shader_file;
        std::ifstream f_shader_file;
        // ensures ifstream objects can throw exceptions:
        v_shader_file.exceptions(std::ifstream::badbit);
        f_shader_file.exceptions(std::ifstream::badbit);
        try {
            // Open files
            v_shader_file.open(vert_path);
            f_shader_file.open(frag_path);
            std::stringstream v_shader_stream, f_shader_stream;
            // Read file's buffer contents into streams
            v_shader_stream << v_shader_file.rdbuf();
            f_shader_stream << f_shader_file.rdbuf();
            // close file handlers
            v_shader_file.close();
            f_shader_file.close();
            // Convert stream into GLchar array
            vertex_code = v_shader_stream.str();
            fragment_code = f_shader_stream.str();
        }
        catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const GLchar* v_shader_code = vertex_code.c_str();
        const GLchar* f_shader_code = fragment_code.c_str();


        // 2. Compile shaders
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];
        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &v_shader_code, NULL);
        glCompileShader(vertex);
        // Print compile errors if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\nin " << vert_path << std::endl;
        }
        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &f_shader_code, NULL);
        glCompileShader(fragment);
        // Print compile errors if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\nin " << frag_path<< std::endl;
        }
        // Shader Program
        this->program_ = glCreateProgram();
        glAttachShader(this->program_, vertex);
        glAttachShader(this->program_, fragment);
        glLinkProgram(this->program_);
        // Print linking errors if any
        glGetProgramiv(this->program_, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->program_, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // Delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);


    }

    void Shader::use() const { glUseProgram(this->program_); }

}
