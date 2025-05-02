#pragma once

#include "cbs_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace cbs {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		CbsCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
}  // namespace cbs