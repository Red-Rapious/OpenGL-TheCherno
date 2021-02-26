#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

struct ShaderProgramSource
{
    //Struct containing both the Fragment and Vertex source code

    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filePath)
{
    //Function which open a file and return a ShaderProgramSource containing the source code
    // of the Fragment and Vertex shaders in this file

    std::ifstream stream(filePath); // file
    
    enum class ShaderType // The type of shader of the present line
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2]; // the strings containing the source code ; for indexes, see ShaderType
    ShaderType type = ShaderType::NONE;
    
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos) // check if the line contains our instruction to delimitate shaders
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX; // set mode to VERTEX
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT; // set mode to SHADER
        }
        else // otherwise, save the line into the proper stringstream
        {
            if ((int)type != -1)
                ss[(int)type] << line << "\n"; // index is determined by the current shader type
            else
                std::cout << "[Shader Error] Undefinied shader type" << std::endl;
        }
    }

    return { ss[0].str(), ss[1].str() }; // return a ShaderProgramSource containing the two stringstreams, converted to strings
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    // Function that compile a shader of 'type' type, and return the id of the compiled shader

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id); // compile shader

    // Error tracking
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)malloc(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type==GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader..." << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}


static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // Create a shader from the two source codes
    // Compile, attach, link and validate the shaders, and return the program id

    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); 

    /* Initializing GLEW */
    if (glewInit() != GLEW_OK)
        std::cout << "Error while initializing GLEW" << std::endl;


    std::cout << "Running on " << glGetString(GL_VERSION) << std::endl; // Print the version
    
    { // scope to avoid getting repeated errors because of the lack of glContext
        /* Vertex positions */
        float positions[] = {
            -0.5f, -0.5f, // 0
             0.5f, -0.5f, // 1
             0.5f,  0.5f, // 2
            -0.5f,  0.5f  // 3
        };

        /* Index buffer: vertex positions needed for each triangle */
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        /* Vertex array object */
        unsigned int vao;
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));

        /*Create an array buffer for positions */
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));

        /* Define the vertex attributes array */
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0)); // Define an array of generic vertex attributes = layout of the vertex buffer


        IndexBuffer ib(indices, 6);

        ShaderProgramSource source = ParseShader("res/shaders/Basic.shader"); // load shader file
        unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource); // compile, attach, link shaders
        GLCall(glUseProgram(shader));

        /* Uniform color management*/
        GLCall(int location = glGetUniformLocation(shader, "u_Color"));
        ASSERT(location != -1);
        GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));
        float r = 0.0f;
        float increment = 0.05f;

        /* Unbind everything */
        GLCall(glBindVertexArray(0));
        GLCall(glUseProgram(0));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {

            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

            //glDrawArrays(GL_TRIANGLES, 0, 3*2); // without index buffer
            GLCall(glUseProgram(shader));
            GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

            GLCall(glBindVertexArray(vao));
            ib.Bind();

            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            // Change color every frame
            if (r >= 1.0f)
                increment = -0.05f;
            else if (r <= 0.0f)
                increment = 0.05f;
            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();

        }

        GLCall(glDeleteProgram(shader));

    }
    glfwTerminate();
    return 0;
}