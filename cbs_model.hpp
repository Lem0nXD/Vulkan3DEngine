#pragma once

#include "cbs_buffer.hpp"
#include "cbs_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace cbs {
    class CbsModel {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal &&
                    uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        CbsModel(CbsDevice& device, const CbsModel::Builder& builder);
        ~CbsModel();

        CbsModel(const CbsModel&) = delete;
        CbsModel& operator=(const CbsModel&) = delete;

        static std::unique_ptr<CbsModel> createModelFromFile(
            CbsDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        uint32_t getVertexCount() { return vertexCount; };

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        CbsDevice& cbsDevice;

        std::unique_ptr<CbsBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<CbsBuffer> indexBuffer;
        uint32_t indexCount;
    };
}  // namespace cbs