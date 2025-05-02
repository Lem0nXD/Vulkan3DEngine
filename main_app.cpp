#include "main_app.hpp"
#include "main_imgui.hpp"
#include "keyboard_movement_controller.hpp"
#include "cbs_buffer.hpp"
#include "cbs_camera.hpp"
#include "render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>


namespace cbs {

    struct GlobalUbo {
        glm::mat4 projectionView{ 1.f };
        glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
    };

    MainApp::MainApp() {
        globalPool =
            CbsDescriptorPool::Builder(cbsDevice)
            .setMaxSets(CbsSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CbsSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    MainApp::~MainApp() {}

    void MainApp::run() {
        std::vector<std::unique_ptr<CbsBuffer>> uboBuffers(CbsSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<CbsBuffer>(
                cbsDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            CbsDescriptorSetLayout::Builder(cbsDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(CbsSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            CbsDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        MainImgui mainImgui{
            cbsWindow,
            cbsDevice,
            cbsRenderer.getSwapChainRenderPass(),
            cbsRenderer.getImageCount()
        };

        RenderSystem RenderSystem{
            cbsDevice,
            cbsRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };
        CbsCamera camera{};

        auto viewerObject = CbsGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        
      

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!cbsWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(cbsWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = cbsRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f); //fov

            if (auto commandBuffer = cbsRenderer.beginFrame()) {
                int frameIndex = cbsRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex] };

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                mainImgui.newFrame();

                cbsRenderer.beginSwapChainRenderPass(commandBuffer);
                RenderSystem.renderGameObjects(frameInfo, gameObjects);

                mainImgui.runGUI(cbsWindow);

                mainImgui.render(commandBuffer);

                cbsRenderer.endSwapChainRenderPass(commandBuffer);
                cbsRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(cbsDevice.device());
    }

    void MainApp::loadGameObjects() {
        std::shared_ptr<CbsModel> cbsModel =
            CbsModel::createModelFromFile(cbsDevice, "models/smooth_vase.obj");
        auto model = CbsGameObject::createGameObject();
        model.model = cbsModel;
        model.transform.translation = { .0f, .25f, 1.5f };//position
        model.transform.scale = { 1.f, 1.f, 1.f };//scale
        gameObjects.push_back(std::move(model));
    }
}  // namespace cbs