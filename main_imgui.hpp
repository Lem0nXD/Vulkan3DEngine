#pragma once

#include "cbs_device.hpp"
#include "cbs_window.hpp"
#include "cbs_pipeline.hpp"

// libs
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

// std
#include <iostream>
#include <stdexcept>

namespace cbs {

	static void check_vk_result(VkResult err) {
		if (err == 0) return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0) abort();
	}

	class MainImgui {
	public:
		MainImgui(
			CbsWindow& window,
			CbsDevice& device,
			VkRenderPass renderPass,
			uint32_t imageCount);
		~MainImgui();

		void newFrame();

		void render(VkCommandBuffer commandBuffer);
		//bool show_demo_window = true;
		bool show_another_window = false;
		bool show_about_window = false;
		bool show_benchmark_window = true;
		bool show_help_window = false;
		bool show_avg_delay_window = false;
		bool show_toolbar_window = true;
		bool show_status_bar = true;

		bool switch_to_normal = true;
		bool switch_to_wireframe = false;
		
		void runGUI(CbsWindow& window);
	private:
		CbsDevice& cbsDevice;

		VkDescriptorPool descriptorPool;
	};
}  // namespace cbs