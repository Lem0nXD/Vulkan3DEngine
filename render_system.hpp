#pragma once

#include "cbs_camera.hpp"
#include "cbs_device.hpp"
#include "cbs_frame_info.hpp"
#include "cbs_game_object.hpp"
#include "cbs_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace cbs {
    class RenderSystem {
    public:
        RenderSystem(
            CbsDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderSystem();

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        void renderGameObjects(FrameInfo& frameInfo, std::vector<CbsGameObject>& gameObjects);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        CbsDevice& cbsDevice;

        std::unique_ptr<CbsPipeline> cbsPipeline;
        VkPipelineLayout pipelineLayout;
    };
}  // namespace cbs