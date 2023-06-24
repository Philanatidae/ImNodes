#include <stdlib.h>
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>

int main() {
    if(!glfwInit()) {
        std::cerr << "Failed to init" << std::endl;
        return EXIT_FAILURE;
    }

    std::atexit(glfwTerminate);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImNodes", nullptr, nullptr);
    if(window == nullptr) {
        std::cerr << "Failed to create window" << std::endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glClearColor(1, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
