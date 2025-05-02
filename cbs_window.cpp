#include "cbs_window.hpp"

// std
#include <stdexcept>

namespace cbs {

    CbsWindow::CbsWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
        initWindow();
    }

    CbsWindow::~CbsWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void CbsWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void CbsWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void CbsWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto cbsWindow = reinterpret_cast<CbsWindow*>(glfwGetWindowUserPointer(window));
        cbsWindow->framebufferResized = true;
        cbsWindow->width = width;
        cbsWindow->height = height;
    }

}  // namespace cbs