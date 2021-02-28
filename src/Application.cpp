#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"

#include "glm//glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Setting profile to CORE to use vertex arrays objects */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL); // 640, 480
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Set the swap interval to make the render smooth */
    glfwSwapInterval(1); 

    /* Initializing GLEW */
    if (glewInit() != GLEW_OK)
        std::cout << "Error while initializing GLEW" << std::endl;


    std::cout << "Running on " << glGetString(GL_VERSION) << std::endl; // Print the version
    
    { // scope to avoid getting repeated errors because of the lack of glContext
        
        /* Vertex positions */
        float positions[] = {
            -0.5f, -0.5f,  0.0f,  0.0f, // 0
             0.5f, -0.5f,  1.0f,  0.0f, // 1
             0.5f,  0.5f,  1.0f,  1.0f, // 2
            -0.5f,  0.5f,  0.0f,  1.0f  // 3
        };

        /* Index buffer: vertex positions needed for each triangle */
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexArray va;
        VertexBuffer vb(positions, 4 * 4 * sizeof(float)); // number of vertices stored * floats per vertex
        VertexBufferLayout layout;
        
        layout.Push<float>(2); // set the layout to a couple of floats
        layout.Push<float>(2); // set the layout to a couple of floats
        va.AddBuffer(vb, layout); // give the layout to opengl

        IndexBuffer ib(indices, 6); // number of vertices to draw


        glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -0.75f, 0.75f, -1.0f, 1.0f); // 4 first values should match the aspect ratio of the window

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();

        /* Uniform color management*/
        float r = 0.0f;
        float increment = 0.05f;
        shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", proj);

        Texture texture("res/textures/logo.png");
        texture.Bind();
        shader.SetUniform1i("u_Texture", 0); // 0 = slot, default value

        /* Unbind everything */
        va.Unbind(); // unbind vertex array
        vb.Unbind(); // unbind array buffer
        ib.Unbind(); // unbind elements array buffer = index buffer
        shader.Unbind();

        Renderer renderer;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            // Change color every frame
            if (r >= 1.0f)
                increment = -0.05f;
            else if (r <= 0.0f)
                increment = 0.05f;
            r += increment;

            // Draw Call
            renderer.Draw(va, ib, shader);


            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();

        }

    } // end of the scope, call the destructors of every object which automatically calls glDeleteProgram

    glfwTerminate();
    return 0;
}