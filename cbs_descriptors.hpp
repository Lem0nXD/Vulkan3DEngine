#pragma once

#include "cbs_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace cbs {

    class CbsDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(CbsDevice& cbsDevice) : cbsDevice{ cbsDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<CbsDescriptorSetLayout> build() const;

        private:
            CbsDevice& cbsDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        CbsDescriptorSetLayout(
            CbsDevice& cbsDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~CbsDescriptorSetLayout();
        CbsDescriptorSetLayout(const CbsDescriptorSetLayout&) = delete;
        CbsDescriptorSetLayout& operator=(const CbsDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        CbsDevice& cbsDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class CbsDescriptorWriter;
    };

    class CbsDescriptorPool {
    public:
        class Builder {
        public:
            Builder(CbsDevice& cbsDevice) : cbsDevice{ cbsDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<CbsDescriptorPool> build() const;

        private:
            CbsDevice& cbsDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        CbsDescriptorPool(
            CbsDevice& cbsDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~CbsDescriptorPool();
        CbsDescriptorPool(const CbsDescriptorPool&) = delete;
        CbsDescriptorPool& operator=(const CbsDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        CbsDevice& cbsDevice;
        VkDescriptorPool descriptorPool;

        friend class CbsDescriptorWriter;
    };

    class CbsDescriptorWriter {
    public:
        CbsDescriptorWriter(CbsDescriptorSetLayout& setLayout, CbsDescriptorPool& pool);

        CbsDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        CbsDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        CbsDescriptorSetLayout& setLayout;
        CbsDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace cbs