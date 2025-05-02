#pragma once

#include "cbs_device.hpp"
#include "cbs_swap_chain.hpp"

// std
#include <string>
#include <vector>
#include <iostream>

namespace cbs {

    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class CbsPipeline {
    public:
        CbsPipeline(
            CbsDevice& device,
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);
        ~CbsPipeline();

        CbsPipeline(const CbsPipeline&) = delete;
        CbsPipeline& operator=(const CbsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        int pipeline_num_switch = 0;
        void setPipelineWireframe() { pipeline_num_switch = 1; }
        void setPipelineNormal() { pipeline_num_switch = 0; }

    private:
        static std::vector<char> readFile(const std::string& filepath);
        void createGraphicsPipeline(
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        CbsDevice& cbsDevice;
        VkPipeline graphicsPipeline;
        VkPipeline wireframePipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule; 
    };
}  // namespace cbs