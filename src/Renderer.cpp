#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] Error code: " << error << " (0x" << std::hex << error << ") from function " << function << " in " << file << " on line " << line << std::endl;
        return false;
    }
    return true;
}