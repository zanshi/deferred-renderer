//
// Created by Niclas Olmenius on 2016-10-27.
//


#pragma once

class ShaderPipeline {

public:
    ShaderPipeline();
    void use_program(GLuint program,  GLbitfield shader_type);
    void use();

private:
    GLuint pipeline_;

};


