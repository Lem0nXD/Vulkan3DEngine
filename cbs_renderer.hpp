#pragma once

#include "cbs_device.hpp"
#include "cbs_swap_chain.hpp"
#include "cbs_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace cbs {
    class CbsRenderer {
    public:
        CbsRenderer(CbsWindow& window, CbsDevice& device);
        ~CbsRenderer();

        CbsRenderer(const CbsRenderer&) = delete;
        CbsRenderer& operator=(const CbsRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return cbsSwapChain->getRenderPass(); }
        uint32_t getImageCount() const { return cbsSwapChain->imageCount(); }//imgui
        float getAspectRatio() const { return cbsSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        CbsWindow& cbsWindow;
        CbsDevice& cbsDevice;
        std::unique_ptr<CbsSwapChain> cbsSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{ 0 };
        bool isFrameStarted{ false };
    };
}  // namespace cbs